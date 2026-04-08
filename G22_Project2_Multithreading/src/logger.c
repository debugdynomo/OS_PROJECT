/*
 * logger.c — STUB implementation of thread-safe logging
 * Owner: Member 5 (Error Handling & Logging)
 *
 * ╔══════════════════════════════════════════════════════════════╗
 * ║  STUB — This file compiles but only prints to stderr.      ║
 * ║  Member 5: Replace the function bodies below with your     ║
 * ║  full implementation (mutex-protected file writes, etc.)    ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include "logger.h"
#include "common.h"

/* ── Member 5: add your static variables here ──────────────────
 *   e.g.  static FILE *log_fp = NULL;
 *         static pthread_mutex_t log_mutex;
 * ────────────────────────────────────────────────────────────── */

static FILE            *log_fp    = NULL;
static pthread_mutex_t  log_mutex = PTHREAD_MUTEX_INITIALIZER;

int logger_init(const char *log_file_path)
{
    /* Member 5: initialise mutex, open log file, etc. */
    pthread_mutex_lock(&log_mutex);

    log_fp = fopen(log_file_path, "a");
    if (!log_fp) {
        perror("[logger] Failed to open log file");
        pthread_mutex_unlock(&log_mutex);
        return -1;
    }
    fprintf(stderr, "[logger] Logging to %s\n", log_file_path);

    pthread_mutex_unlock(&log_mutex);
    return 0;
}

void log_operation(const char *operation,
                   const char *filepath,
                   const char *status,
                   const char *detail)
{
    /* Member 5: replace with full timestamped, thread-tagged logging */
    pthread_mutex_lock(&log_mutex);

    /* Get timestamp */
    time_t     now  = time(NULL);
    struct tm *tm_info = localtime(&now);
    char       time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    /* Get thread ID */
    pthread_t tid = pthread_self();

    const char *det = detail ? detail : "";

    /* Write to log file */
    if (log_fp) {
        fprintf(log_fp, "[%s] [TID:%lu] [%s] %s — %s — %s\n",
                time_buf, (unsigned long)tid, operation,
                filepath, status, det);
        fflush(log_fp);
    }

    /* Also print to stderr for visibility */
    fprintf(stderr, "[%s] [TID:%lu] [%s] %s — %s — %s\n",
            time_buf, (unsigned long)tid, operation,
            filepath, status, det);

    pthread_mutex_unlock(&log_mutex);
}

void logger_close(void)
{
    /* Member 5: flush, close, destroy mutex */
    pthread_mutex_lock(&log_mutex);
    if (log_fp) {
        fflush(log_fp);
        fclose(log_fp);
        log_fp = NULL;
    }
    pthread_mutex_unlock(&log_mutex);
    pthread_mutex_destroy(&log_mutex);
    fprintf(stderr, "[logger] Logger closed.\n");
}
