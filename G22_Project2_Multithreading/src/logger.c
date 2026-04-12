#include "logger.h"
#include "common.h"

static FILE *log_fp = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

int logger_init(const char *log_file_path)
{
    pthread_mutex_lock(&log_mutex);
    log_fp = fopen(log_file_path, "a");
    if (!log_fp) {
        pthread_mutex_unlock(&log_mutex);
        return -1;
    }
    pthread_mutex_unlock(&log_mutex);
    return 0;
}

void log_operation(const char *operation, const char *filepath, const char *status, const char *detail)
{
    pthread_mutex_lock(&log_mutex);

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    pthread_t tid = pthread_self();
    const char *det = detail ? detail : "";

    if (log_fp) {
        fprintf(log_fp, "[%s] [TID:%lu] [%s] %s - %s - %s\n",
                time_buf, (unsigned long)tid, operation, filepath, status, det);
        fflush(log_fp);
    }

    fprintf(stderr, "[%s] [TID:%lu] [%s] %s - %s - %s\n",
            time_buf, (unsigned long)tid, operation, filepath, status, det);
#include <time.h>
#include <unistd.h>
#include <string.h>

FILE *log_file;
pthread_mutex_t log_mutex;

void logger_init(const char *filename) {
    log_file = fopen(filename, "a");
    if (!log_file) {
        perror("Failed to open log file");
        return;
    }
    pthread_mutex_init(&log_mutex, NULL);
}

void log_operation(const char *operation, const char *filename, const char *status, const char *details) {
    pthread_mutex_lock(&log_mutex);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(log_file,
        "[%04d-%02d-%02d %02d:%02d:%02d] [TID:%ld] [%s] %s - %s - %s\n",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec,
        pthread_self(),
        operation,
        filename,
        status,
        details
    );

    fflush(log_file);

    pthread_mutex_unlock(&log_mutex);
}

<<<<<<< HEAD
void logger_close(void)
{
    pthread_mutex_lock(&log_mutex);
    if (log_fp) {
        fflush(log_fp);
        fclose(log_fp);
        log_fp = NULL;
    }
    pthread_mutex_unlock(&log_mutex);
    pthread_mutex_destroy(&log_mutex);
=======
void logger_close() {
    pthread_mutex_destroy(&log_mutex);
    fclose(log_file);
>>>>>>> origin/project2-saiteja
}
