#define _GNU_SOURCE

#include "common.h"
#include "sync.h"
#include "signals.h"
#include "logger.h"
#include "file_ops.h"
#include "compression.h"

static int             active_threads = 0;
static pthread_mutex_t counter_mutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  counter_cond   = PTHREAD_COND_INITIALIZER;

static void increment_active(void)
{
    pthread_mutex_lock(&counter_mutex);
    active_threads++;
    pthread_mutex_unlock(&counter_mutex);
}

static void decrement_active(void)
{
    pthread_mutex_lock(&counter_mutex);
    active_threads--;
    if (active_threads == 0)
        pthread_cond_signal(&counter_cond);
    pthread_mutex_unlock(&counter_mutex);
}

typedef struct {
    void *(*func)(void *);
    void  *arg;
} wrapper_arg_t;

static void *thread_wrapper(void *raw)
{
    wrapper_arg_t *wa = (wrapper_arg_t *)raw;
    void *(*func)(void *) = wa->func;
    void  *arg            = wa->arg;
    free(wa);

    func(arg);
    decrement_active();
    return NULL;
}

static int spawn_thread(void *(*func)(void *), void *arg)
{
    wrapper_arg_t *wa = malloc(sizeof(wrapper_arg_t));
    if (!wa) { perror("malloc"); return -1; }
    wa->func = func;
    wa->arg  = arg;

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    increment_active();
    int rc = pthread_create(&tid, &attr, thread_wrapper, wa);
    pthread_attr_destroy(&attr);

    if (rc != 0) {
        fprintf(stderr, "pthread_create failed: %s\n", strerror(rc));
        decrement_active();
        free(wa);
        return -1;
    }
    return 0;
}

static void wait_all_threads(void)
{
    pthread_mutex_lock(&counter_mutex);
    printf("Waiting for %d active thread(s) to finish...\n", active_threads);
    while (active_threads > 0)
        pthread_cond_wait(&counter_cond, &counter_mutex);
    pthread_mutex_unlock(&counter_mutex);
    printf("All threads finished.\n");
}

static void print_menu(void)
{
    printf("\n");
    printf("================================================\n");
    printf("     MULTITHREADED FILE MANAGEMENT SYSTEM       \n");
    printf("================================================\n");
    printf("  1. Read File                                  \n");
    printf("  2. Write to File                              \n");
    printf("  3. Delete File                                \n");
    printf("  4. Rename File                                \n");
    printf("  5. Copy File                                  \n");
    printf("  6. Display File Metadata                      \n");
    printf("  7. Compress File (gzip)                       \n");
    printf("  8. Decompress File (gzip)                     \n");
    printf("  9. View Log File                              \n");
    printf(" 10. Exit                                       \n");
    printf("================================================\n");
    printf("  Active threads: %d\n", active_threads);
    printf("  Enter choice: ");
    fflush(stdout);
}

static void read_line(const char *prompt, char *buf, int size)
{
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buf, size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n')
        buf[len - 1] = '\0';
}

static void do_read(void)
{
    file_arg_t *arg = malloc(sizeof(file_arg_t));
    if (!arg) { perror("malloc"); return; }
    read_line("  Enter file path to read: ", arg->filepath, MAX_PATH_LEN);

    if (spawn_thread(thread_read_file, arg) != 0) {
        free(arg);
        return;
    }
    printf("  -> Thread spawned for READ.\n");
}

static void do_write(void)
{
    write_arg_t *arg = malloc(sizeof(write_arg_t));
    if (!arg) { perror("malloc"); return; }

    read_line("  Enter file path to write: ", arg->filepath, MAX_PATH_LEN);

    char mode[8];
    read_line("  Append or overwrite? (a/o): ", mode, sizeof(mode));
    arg->append = (mode[0] == 'a' || mode[0] == 'A') ? 1 : 0;

    read_line("  Enter content: ", arg->content, BUFFER_SIZE);
    strncat(arg->content, "\n", BUFFER_SIZE - strlen(arg->content) - 1);

    if (spawn_thread(thread_write_file, arg) != 0) {
        free(arg);
        return;
    }
    printf("  -> Thread spawned for WRITE.\n");
}

static void do_delete(void)
{
    file_arg_t *arg = malloc(sizeof(file_arg_t));
    if (!arg) { perror("malloc"); return; }
    read_line("  Enter file path to delete: ", arg->filepath, MAX_PATH_LEN);

    if (spawn_thread(thread_delete_file, arg) != 0) {
        free(arg);
        return;
    }
    printf("  -> Thread spawned for DELETE.\n");
}

static void do_rename(void)
{
    rename_arg_t *arg = malloc(sizeof(rename_arg_t));
    if (!arg) { perror("malloc"); return; }
    read_line("  Enter current file path: ", arg->old_path, MAX_PATH_LEN);
    read_line("  Enter new file path:     ", arg->new_path, MAX_PATH_LEN);

    if (spawn_thread(thread_rename_file, arg) != 0) {
        free(arg);
        return;
    }
    printf("  -> Thread spawned for RENAME.\n");
}

static void do_copy(void)
{
    copy_arg_t *arg = malloc(sizeof(copy_arg_t));
    if (!arg) { perror("malloc"); return; }
    read_line("  Enter source file path: ", arg->src_path, MAX_PATH_LEN);
    read_line("  Enter destination path: ", arg->dst_path, MAX_PATH_LEN);

    if (spawn_thread(thread_copy_file, arg) != 0) {
        free(arg);
        return;
    }
    printf("  -> Thread spawned for COPY.\n");
}

static void do_metadata(void)
{
    file_arg_t *arg = malloc(sizeof(file_arg_t));
    if (!arg) { perror("malloc"); return; }
    read_line("  Enter file path: ", arg->filepath, MAX_PATH_LEN);

    if (spawn_thread(thread_display_metadata, arg) != 0) {
        free(arg);
        return;
    }
    printf("  -> Thread spawned for METADATA.\n");
}

static void do_compress(void)
{
    file_arg_t *arg = malloc(sizeof(file_arg_t));
    if (!arg) { perror("malloc"); return; }
    read_line("  Enter file path to compress: ", arg->filepath, MAX_PATH_LEN);

    if (spawn_thread(thread_compress_file, arg) != 0) {
        free(arg);
        return;
    }
    printf("  -> Thread spawned for COMPRESS.\n");
}

static void do_decompress(void)
{
    file_arg_t *arg = malloc(sizeof(file_arg_t));
    if (!arg) { perror("malloc"); return; }
    read_line("  Enter .gz file path to decompress: ", arg->filepath, MAX_PATH_LEN);

    if (spawn_thread(thread_decompress_file, arg) != 0) {
        free(arg);
        return;
    }
    printf("  -> Thread spawned for DECOMPRESS.\n");
}

static void do_view_logs(void)
{
    FILE *fp = fopen(LOG_FILE_PATH, "r");
    if (!fp) {
        printf("  No log file found at '%s'.\n", LOG_FILE_PATH);
        return;
    }
    printf("\n--- Log entries ---\n");
    char buf[1024];
    while (fgets(buf, sizeof(buf), fp)) {
        printf("  %s", buf);
    }
    printf("--- End of logs ---\n");
    fclose(fp);
}

int main(void)
{
    setup_signal_handlers();

    lock_table_init();
    if (logger_init(LOG_FILE_PATH) != 0) {
        fprintf(stderr, "Warning: logger initialization failed.\n");
    }

    log_operation("SYSTEM", "-", "START", "Started");

    while (!g_shutdown) {
        print_menu();

        char input[16];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        int choice = atoi(input);

        switch (choice) {
        case 1:  do_read();        break;
        case 2:  do_write();       break;
        case 3:  do_delete();      break;
        case 4:  do_rename();      break;
        case 5:  do_copy();        break;
        case 6:  do_metadata();    break;
        case 7:  do_compress();    break;
        case 8:  do_decompress();  break;
        case 9:  do_view_logs();   break;
        case 10:
            g_shutdown = 1;
            break;
        default:
            printf("  Invalid choice. Try again.\n");
        }

        usleep(100000);
    }

    printf("\nShutting down...\n");
    wait_all_threads();
    log_operation("SYSTEM", "-", "STOP", "Stopped");
    logger_close();
    lock_table_destroy();

    printf("Done.\n");
}
