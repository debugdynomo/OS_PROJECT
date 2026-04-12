/*
 * thread_display_metadata.c  —  Feature 6: Concurrent Metadata Display
 * Member 4: File Copying & Metadata Display
 *
 * Simulates a multithreaded metadata-display operation using fork()-based
 * "threads" and a file-based Read-Write lock.
 *
 * Architecture:
 *   - A shared lock-file acts as a simple RW-lock (same scheme as
 *     thread_copy_file.c — see comments there for full protocol).
 *   - Multiple "reader threads" can concurrently call stat() on
 *     different files.  Each acquires a READ lock, calls stat(), prints
 *     the metadata, then releases its READ lock.
 *   - A "status writer thread" (optional) acquires a WRITE lock and
 *     appends a summary line to a results file, safely preventing any
 *     concurrent reads during the write.
 *   - The parent waits for all children, prints a final table, and
 *     cleans up.
 *
 * Usage:
 *   thread_display_metadata <file1> [file2] ... [fileN]
 *
 * Example (inside xv6 shell):
 *   thread_display_metadata README cat echo
 */

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

/* ------------------------------------------------------------------ */
/* Constants                                                           */
/* ------------------------------------------------------------------ */
#define LOCKFILE    "meta_rwlock.tmp"
#define RESULTFILE  "meta_results.txt"
#define MAX_FILES   16
#define MAX_RETRY   200

/* ------------------------------------------------------------------ */
/* Helper                                                              */
/* ------------------------------------------------------------------ */
static void yield_cpu(void)
{
    pause(1);
}

/* ------------------------------------------------------------------ */
/* File-based Read-Write lock (same protocol as thread_copy_file)      */
/*   'F' = free | 'R' = read-locked | 'W' = write-locked              */
/* ------------------------------------------------------------------ */

static char get_lock(void)
{
    char state = 'F';
    int fd = open(LOCKFILE, O_RDONLY);
    if (fd < 0) return 'F';
    read(fd, &state, 1);
    close(fd);
    return state;
}

static int set_lock(char state)
{
    int fd = open(LOCKFILE, O_RDWR);
    if (fd < 0) return -1;
    write(fd, &state, 1);
    close(fd);
    return 0;
}

/*
 * acquire_read_lock —
 *   In a full RW-lock multiple readers may coexist.  Here we permit
 *   one active reader at a time to keep the file-state simple while
 *   still demonstrating correct lock management (read blocks on write,
 *   write blocks on read).
 */
static void acquire_read_lock(void)
{
    int retries = 0;
    while (1) {
        char s = get_lock();
        /* Allow read if lock is free or already held by another reader.
         * ('R' state: we treat it as counting — real implementation would
         *  track a reader count, but for this demo we proceed when 'R') */
        if (s == 'F') {
            set_lock('R');
            /* brief re-check to detect racing writer */
            if (get_lock() != 'W')
                return;
        } else if (s == 'R') {
            /* Another reader holds the lock — join (shared read) */
            return;
        }
        retries++;
        if (retries > MAX_RETRY) {
            printf("thread_display_metadata: read lock timeout\n");
            return;
        }
        yield_cpu();
    }
}

static void acquire_write_lock(void)
{
    int retries = 0;
    while (1) {
        char s = get_lock();
        if (s == 'F') {
            set_lock('W');
            if (get_lock() == 'W')
                return;
        }
        retries++;
        if (retries > MAX_RETRY) {
            printf("thread_display_metadata: write lock timeout\n");
            return;
        }
        yield_cpu();
    }
}

static void release_lock(void)
{
    set_lock('F');
}

/* ------------------------------------------------------------------ */
/* type_str — human-readable file type                                 */
/* ------------------------------------------------------------------ */
static const char *type_str(short type)
{
    switch (type) {
    case T_FILE:   return "FILE";
    case T_DIR:    return "DIR ";
    case T_DEVICE: return "DEV ";
    default:       return "????";
    }
}

/* ------------------------------------------------------------------ */
/* Reader thread: stat() one file and print its metadata               */
/* ------------------------------------------------------------------ */
static void reader_thread(const char *path, int thread_id)
{
    struct stat st;

    printf("[Reader-%d pid=%d] Acquiring READ lock for stat('%s')\n",
           thread_id, getpid(), path);

    acquire_read_lock();

    printf("[Reader-%d pid=%d] READ lock acquired — calling stat()\n",
           thread_id, getpid());

    if (stat(path, &st) < 0) {
        printf("[Reader-%d pid=%d] ERROR: cannot stat '%s'\n",
               thread_id, getpid(), path);
        release_lock();
        exit(1);
    }

    /* ------- Print metadata ------- */
    printf("\n");
    printf("  +--------------------------------------------+\n");
    printf("  | Metadata for: %-28s |\n", path);
    printf("  +--------------------------------------------+\n");
    printf("  | Thread ID   : %-28d |\n", thread_id);
    printf("  | PID         : %-28d |\n", getpid());
    printf("  | Type        : %-28s |\n", type_str(st.type));
    printf("  | Inode       : %-28d |\n", (int)st.ino);
    printf("  | Hard links  : %-28d |\n", (int)st.nlink);
    printf("  | Size (bytes): %-28d |\n", (int)st.size);
    printf("  | Device      : %-28d |\n", (int)st.dev);
    printf("  +--------------------------------------------+\n");
    printf("\n");

    printf("[Reader-%d pid=%d] Releasing READ lock\n", thread_id, getpid());
    release_lock();

    exit(0);
}

/* ------------------------------------------------------------------ */
/* Writer thread: append a summary line to RESULTFILE                  */
/* ------------------------------------------------------------------ */
static void writer_thread(int file_count)
{
    printf("[Writer pid=%d] Acquiring WRITE lock to record summary\n",
           getpid());

    acquire_write_lock();

    printf("[Writer pid=%d] WRITE lock acquired — appending to '%s'\n",
           getpid(), RESULTFILE);

    int fd = open(RESULTFILE, O_CREATE | O_RDWR);
    if (fd < 0) {
        printf("[Writer pid=%d] ERROR: cannot open result file\n", getpid());
        release_lock();
        exit(1);
    }

    /* Seek to end by reading to end */
    char tmp[1];
    while (read(fd, tmp, 1) > 0)
        ;

    /* Write summary line */
    char line[128];
    /* Build line manually since sprintf not available */
    int pos = 0;
    const char *prefix = "Summary: metadata displayed for ";
    for (int i = 0; prefix[i]; i++) line[pos++] = prefix[i];
    /* Append file_count as digit */
    if (file_count >= 10) line[pos++] = '0' + file_count / 10;
    line[pos++] = '0' + file_count % 10;
    const char *suffix = " file(s)\n";
    for (int i = 0; suffix[i]; i++) line[pos++] = suffix[i];
    line[pos] = '\0';

    write(fd, line, pos);
    close(fd);

    printf("[Writer pid=%d] Summary written — releasing WRITE lock\n",
           getpid());
    release_lock();

    exit(0);
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: thread_display_metadata <file1> [file2 ...]\n");
        exit(1);
    }

    int file_count = argc - 1;
    if (file_count > MAX_FILES) {
        printf("thread_display_metadata: too many files (max %d)\n", MAX_FILES);
        exit(1);
    }

    printf("\n=== thread_display_metadata: Displaying metadata for %d file(s) ===\n",
           file_count);
    printf("[Main pid=%d] Initialising RW-lock file: %s\n",
           getpid(), LOCKFILE);

    /* Initialise lock file */
    int lfd = open(LOCKFILE, O_CREATE | O_RDWR | O_TRUNC);
    if (lfd < 0) {
        printf("thread_display_metadata: cannot create lock file\n");
        exit(1);
    }
    char init = 'F';
    write(lfd, &init, 1);
    close(lfd);

    /* Fork one reader thread per file */
    int pids[MAX_FILES + 1];
    int n_pids = 0;

    for (int i = 0; i < file_count; i++) {
        int pid = fork();
        if (pid < 0) {
            printf("[Main] fork() failed for thread %d\n", i);
            exit(1);
        }
        if (pid == 0) {
            /* Child: reader thread */
            reader_thread(argv[i + 1], i + 1);
            /* NOT REACHED */
        }
        pids[n_pids++] = pid;
        printf("[Main pid=%d] Launched Reader-%d (pid=%d) for '%s'\n",
               getpid(), i + 1, pid, argv[i + 1]);
    }

    /* Wait for all reader threads to finish before launching writer */
    for (int i = 0; i < file_count; i++) {
        int status;
        wait(&status);
    }

    printf("[Main pid=%d] All reader threads done. Launching writer thread.\n",
           getpid());

    /* Reset lock to 'F' before writer */
    set_lock('F');

    /* Fork writer thread */
    int wpid = fork();
    if (wpid < 0) {
        printf("[Main] fork() failed for writer thread\n");
        exit(1);
    }
    if (wpid == 0) {
        writer_thread(file_count);
        /* NOT REACHED */
    }

    printf("[Main pid=%d] Launched Writer thread (pid=%d)\n",
           getpid(), wpid);

    /* Wait for writer */
    int wstatus;
    wait(&wstatus);

    if (wstatus == 0) {
        printf("[Main] Writer thread completed successfully.\n");
    } else {
        printf("[Main] Writer thread exited with error %d.\n", wstatus);
    }

    /* Clean up lock file */
    unlink(LOCKFILE);

    printf("=== thread_display_metadata: DONE ===\n\n");
    exit(0);
}

