#include "file_ops.h"
#include "sync.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

static const char *errmsg(void) {
    return strerror(errno);
}

static void simulate_io_delay_ms(int ms) {
    struct timespec ts = {
        .tv_sec  = ms / 1000,
        .tv_nsec = (ms % 1000) * 1000000L
    };
    nanosleep(&ts, NULL);
}


void *thread_read_file(void *arg) {
    ReadArgs *a = (ReadArgs *)arg;
    int rc = 0;
    char detail[512];

    printf("[Reader %d] Requesting read lock on '%s'\n",
           a->thread_id, a->filename);

    /* Acquire read lock */
    file_read_lock(a->filename);

    printf("[Reader %d] Lock acquired\n", a->thread_id);

    FILE *fp = fopen(a->filename, "r");
    if (!fp) {
        snprintf(detail, sizeof(detail),
                 "fopen failed: %s", errmsg());
        fprintf(stderr, "[Reader %d] ERROR: %s\n", a->thread_id, detail);
        log_operation("READ", a->filename, "ERROR", detail);
        file_unlock(a->filename);
        return (void *)(intptr_t)1;
    }

    printf("[Reader %d] ----- BEGIN FILE -----\n", a->thread_id);

    char buf[1024];
    size_t bytes_read = 0;

    while (fgets(buf, sizeof(buf), fp)) {
        printf("[Reader %d] %s", a->thread_id, buf);
        bytes_read += strlen(buf);
        simulate_io_delay_ms(10);
    }

    if (ferror(fp)) {
        snprintf(detail, sizeof(detail),
                 "read error: %s", errmsg());
        log_operation("READ", a->filename, "ERROR", detail);
        rc = 1;
    } else {
        snprintf(detail, sizeof(detail),
                 "%zu bytes read", bytes_read);
        log_operation("READ", a->filename, "SUCCESS", detail);
    }

    fclose(fp);

    file_unlock(a->filename);

    printf("[Reader %d] Lock released\n", a->thread_id);

    return (void *)(intptr_t)rc;
}

void *thread_write_file(void *arg) {
    WriteArgs *a = (WriteArgs *)arg;
    int rc = 0;
    char detail[512];

    printf("[Writer %d] Requesting write lock on '%s'\n",
           a->thread_id, a->filename);

    file_write_lock(a->filename);

    printf("[Writer %d] Lock acquired\n", a->thread_id);

    const char *mode = (a->mode == WRITE_MODE_APPEND) ? "a" : "w";

    FILE *fp = fopen(a->filename, mode);
    if (!fp) {
        snprintf(detail, sizeof(detail),
                 "fopen failed: %s", errmsg());
        fprintf(stderr, "[Writer %d] ERROR: %s\n", a->thread_id, detail);
        log_operation("WRITE", a->filename, "ERROR", detail);
        file_unlock(a->filename);
        return (void *)(intptr_t)1;
    }

    simulate_io_delay_ms(50);

    size_t len = strlen(a->content);
    size_t written = fwrite(a->content, 1, len, fp);

    if (written != len) {
        snprintf(detail, sizeof(detail),
                 "write error: %s", errmsg());
        log_operation("WRITE", a->filename, "ERROR", detail);
        rc = 1;
    } else {
        if (len > 0 && a->content[len - 1] != '\n')
            fputc('\n', fp);

        snprintf(detail, sizeof(detail),
                 "%zu bytes written", written);
        log_operation("WRITE", a->filename, "SUCCESS", detail);
    }

    fclose(fp);

    file_unlock(a->filename);

    printf("[Writer %d] Lock released\n", a->thread_id);

    return (void *)(intptr_t)rc;
}


void demo_concurrent_reads(const char *filename, int count) {
    pthread_t *tids = malloc(count * sizeof(pthread_t));
    ReadArgs *args = malloc(count * sizeof(ReadArgs));

    for (int i = 0; i < count; i++) {
        args[i].filename = filename;
        args[i].thread_id = i + 1;

        pthread_create(&tids[i], NULL,
                       thread_read_file, &args[i]);
    }

    for (int i = 0; i < count; i++) {
        pthread_join(tids[i], NULL);
    }

    free(tids);
    free(args);
}