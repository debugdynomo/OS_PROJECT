#include "logger.h"
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

void logger_close() {
    pthread_mutex_destroy(&log_mutex);
    fclose(log_file);
}
