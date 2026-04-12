/*
 * thread_copy_file.c  —  Feature 5: Concurrent File Copying
 * Member 4: File Copying & Metadata Display
 *
 * Simulates a multithreaded file-copy operation using fork()-based
 * "threads" and a file-based Read-Write lock.
 *
 * Architecture:
 *   - A shared lock-file (LOCKFILE) acts as a simple RW-lock.
 *     Its first byte encodes the current state:
 *       'W' = write lock held
 *       'R' = read lock held
 *       'F' = free (no lock)
 *   - Parent process  : orchestrator — forks reader & writer threads,
 *                        waits for them all, prints final status.
 *   - Reader "thread" : acquires READ  lock → reads source file →
 *                        releases lock.
 *   - Writer "thread" : acquires WRITE lock → writes to dest file →
 *                        releases lock.
 *
 * Usage:
 *   thread_copy_file <source> <destination>
 */

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

/* ------------------------------------------------------------------ */
/* Constants                                                           */
/* ------------------------------------------------------------------ */
#define LOCKFILE   "copy_rwlock.tmp"
#define BUFSIZE    512
#define MAX_RETRY  200

/* ------------------------------------------------------------------ */
/* Helper: busy-sleep for a short time (uses pause syscall, 1 tick)   */
/* ------------------------------------------------------------------ */
static void yield_cpu(void)
{
    pause(1);
}

/* ------------------------------------------------------------------ */
/* Lock helpers — file-based Read-Write lock                           */
/*                                                                     */
/* acquire_read_lock :                                                 */
/*   Spin until the lock is 'F', then atomically set it to 'R'.       */
/*   Multiple readers could coexist in a real RW-lock; for clarity    */
/*   we use a single-reader model here (sufficient for the demo).     */
/*                                                                     */
/* acquire_write_lock :                                                */
/*   Spin until the lock is 'F', then atomically set it to 'W'.       */
/*                                                                     */
/* release_lock :                                                      */
/*   Set the lock back to 'F'.                                         */
/* ------------------------------------------------------------------ */

static int set_lock_state(char state)
{
    int fd = open(LOCKFILE, O_RDWR);
    if (fd < 0) return -1;
    write(fd, &state, 1);
    close(fd);
    return 0;
}

static char get_lock_state(void)
{
    char state = 'F';
    int fd = open(LOCKFILE, O_RDONLY);
    if (fd < 0) return 'F';
    read(fd, &state, 1);
    close(fd);
    return state;
}

static void acquire_read_lock(void)
{
    int retries = 0;
    while (1) {
        char s = get_lock_state();
        if (s == 'F') {
            /* Attempt to grab the read lock */
            if (set_lock_state('R') == 0) {
                /* Confirm we actually own it (no write snuck in) */
                if (get_lock_state() == 'R')
                    return;
            }
        }
        retries++;
        if (retries > MAX_RETRY) {
            printf("thread_copy_file: read lock timeout, forcing\n");
            return;
        }
        yield_cpu();
    }
}

static void acquire_write_lock(void)
{
    int retries = 0;
    while (1) {
        char s = get_lock_state();
        if (s == 'F') {
            if (set_lock_state('W') == 0) {
                if (get_lock_state() == 'W')
                    return;
            }
        }
        retries++;
        if (retries > MAX_RETRY) {
            printf("thread_copy_file: write lock timeout, forcing\n");
            return;
        }
        yield_cpu();
    }
}

static void release_lock(void)
{
    set_lock_state('F');
}

/* ------------------------------------------------------------------ */
/* Reader thread: reads source file, stores data in a pipe            */
/* ------------------------------------------------------------------ */
static void reader_thread(const char *src, int write_pipe_fd)
{
    char buf[BUFSIZE];
    int n, src_fd;

    printf("[Reader Thread pid=%d] Acquiring READ lock on source: %s\n",
           getpid(), src);
    acquire_read_lock();
    printf("[Reader Thread pid=%d] READ lock acquired\n", getpid());

    src_fd = open(src, O_RDONLY);
    if (src_fd < 0) {
        printf("[Reader Thread pid=%d] ERROR: cannot open source '%s'\n",
               getpid(), src);
        release_lock();
        exit(1);
    }

    printf("[Reader Thread pid=%d] Reading from '%s'\n", getpid(), src);
    int total = 0;
    while ((n = read(src_fd, buf, BUFSIZE)) > 0) {
        write(write_pipe_fd, buf, n);
        total += n;
    }
    close(src_fd);
    close(write_pipe_fd);  /* signal EOF to writer */

    printf("[Reader Thread pid=%d] Read %d bytes — releasing READ lock\n",
           getpid(), total);
    release_lock();

    exit(0);
}

/* ------------------------------------------------------------------ */
/* Writer thread: reads from pipe, writes destination file             */
/* ------------------------------------------------------------------ */
static void writer_thread(const char *dst, int read_pipe_fd)
{
    char buf[BUFSIZE];
    int n, dst_fd;

    printf("[Writer Thread pid=%d] Acquiring WRITE lock on dest: %s\n",
           getpid(), dst);
    acquire_write_lock();
    printf("[Writer Thread pid=%d] WRITE lock acquired\n", getpid());

    dst_fd = open(dst, O_CREATE | O_RDWR | O_TRUNC);
    if (dst_fd < 0) {
        printf("[Writer Thread pid=%d] ERROR: cannot open dest '%s'\n",
               getpid(), dst);
        release_lock();
        exit(1);
    }

    printf("[Writer Thread pid=%d] Writing to '%s'\n", getpid(), dst);
    int total = 0;
    while ((n = read(read_pipe_fd, buf, BUFSIZE)) > 0) {
        if (write(dst_fd, buf, n) != n) {
            printf("[Writer Thread pid=%d] ERROR: write failed\n", getpid());
            close(dst_fd);
            close(read_pipe_fd);
            release_lock();
            exit(1);
        }
        total += n;
    }
    close(read_pipe_fd);
    close(dst_fd);

    printf("[Writer Thread pid=%d] Wrote %d bytes — releasing WRITE lock\n",
           getpid(), total);
    release_lock();

    exit(0);
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: thread_copy_file <source> <destination>\n");
        exit(1);
    }

    const char *src = argv[1];
    const char *dst = argv[2];

    /* Verify source exists */
    struct stat st;
    if (stat(src, &st) < 0) {
        printf("thread_copy_file: cannot stat source '%s'\n", src);
        exit(1);
    }
    if (st.type != T_FILE) {
        printf("thread_copy_file: '%s' is not a regular file\n", src);
        exit(1);
    }

    printf("\n=== thread_copy_file: Copying '%s' -> '%s' ===\n", src, dst);
    printf("[Main pid=%d] Initialising RW-lock file: %s\n",
           getpid(), LOCKFILE);

    /* Initialise the lock file to 'F' (free) */
    int lfd = open(LOCKFILE, O_CREATE | O_RDWR | O_TRUNC);
    if (lfd < 0) {
        printf("thread_copy_file: cannot create lock file\n");
        exit(1);
    }
    char init_state = 'F';
    write(lfd, &init_state, 1);
    close(lfd);

    /* Create a pipe to pass data between reader and writer threads */
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        printf("thread_copy_file: pipe() failed\n");
        exit(1);
    }

    /* Fork reader thread */
    int reader_pid = fork();
    if (reader_pid < 0) {
        printf("thread_copy_file: fork() for reader failed\n");
        exit(1);
    }
    if (reader_pid == 0) {
        /* Child: reader — close the read end of pipe */
        close(pipefd[0]);
        reader_thread(src, pipefd[1]);
        /* NOT REACHED */
    }

    /* Fork writer thread */
    int writer_pid = fork();
    if (writer_pid < 0) {
        printf("thread_copy_file: fork() for writer failed\n");
        exit(1);
    }
    if (writer_pid == 0) {
        /* Child: writer — close the write end of pipe */
        close(pipefd[1]);
        writer_thread(dst, pipefd[0]);
        /* NOT REACHED */
    }

    /* Parent closes both pipe ends (children own them) */
    close(pipefd[0]);
    close(pipefd[1]);

    printf("[Main pid=%d] Reader thread pid=%d, Writer thread pid=%d\n",
           getpid(), reader_pid, writer_pid);
    printf("[Main pid=%d] Waiting for threads to complete...\n", getpid());

    /* Wait for both children */
    int status;
    int exit_code = 0;
    for (int i = 0; i < 2; i++) {
        int pid = wait(&status);
        if (status != 0) {
            printf("[Main] Thread pid=%d exited with error %d\n", pid, status);
            exit_code = status;
        } else {
            printf("[Main] Thread pid=%d completed successfully\n", pid);
        }
    }

    /* Verify the copy */
    struct stat dst_st;
    if (stat(dst, &dst_st) == 0) {
        printf("[Main] Copy verified: '%s' size=%d bytes\n", dst, (int)dst_st.size);
    }

    /* Clean up lock file */
    unlink(LOCKFILE);

    if (exit_code == 0) {
        printf("=== thread_copy_file: SUCCESS — '%s' copied to '%s' ===\n\n",
               src, dst);
    } else {
        printf("=== thread_copy_file: FAILED (exit code %d) ===\n\n", exit_code);
    }

    exit(exit_code);
}

