#include "logger.h"
#include "common.h"

static FILE *log_fp = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

int logger_init(const char *log_file_path)
{
    pthread_mutex_lock(&log_mutex);

    // Create the logs directory if the path contains a directory
    char tmp_path[256];
    strncpy(tmp_path, log_file_path, sizeof(tmp_path) - 1);
    tmp_path[sizeof(tmp_path) - 1] = '\0';
    char *slash = strrchr(tmp_path, '/');
    if (slash) {
        *slash = '\0';
        mkdir(tmp_path, 0777); // sys/stat.h included via common.h
    }

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

    pthread_mutex_unlock(&log_mutex);
}

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
}
