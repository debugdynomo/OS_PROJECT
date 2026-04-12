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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "file_ops.h"
#include "sync.h"
#include "logger.h"

void* thread_delete_file(void* arg) {
    file_args* args = (file_args*)arg;
    char* filename = args->filename;

    pthread_rwlock_t* lock = get_file_lock(filename);

    // Take write lock (exclusive)
    pthread_rwlock_wrlock(lock);

    int res = remove(filename);

    if (res == 0) {
        log_operation("DELETE", filename, "SUCCESS", "File deleted");
        printf("Deleted: %s\n", filename);
    } else {
        log_operation("DELETE", filename, "FAIL", strerror(errno));
        printf("Error deleting %s: %s\n", filename, strerror(errno));
    }

    pthread_rwlock_unlock(lock);
    release_file_lock(filename);

    return NULL;
}


void* thread_rename_file(void* arg) {
    rename_args* args = (rename_args*)arg;

    char* oldname = args->oldname;
    char* newname = args->newname;

    pthread_rwlock_t *lock1, *lock2;

    // Deadlock prevention: alphabetical order
    if (strcmp(oldname, newname) < 0) {
        lock1 = get_file_lock(oldname);
        lock2 = get_file_lock(newname);
    } else {
        lock1 = get_file_lock(newname);
        lock2 = get_file_lock(oldname);
    }

    pthread_rwlock_wrlock(lock1);
    pthread_rwlock_wrlock(lock2);

    int res = rename(oldname, newname);

    if (res == 0) {
        log_operation("RENAME", oldname, "SUCCESS", newname);
        printf("Renamed: %s → %s\n", oldname, newname);
    } else {
        log_operation("RENAME", oldname, "FAIL", strerror(errno));
        printf("Error renaming %s: %s\n", oldname, strerror(errno));
    }

    pthread_rwlock_unlock(lock2);
    pthread_rwlock_unlock(lock1);

    release_file_lock(oldname);
    release_file_lock(newname);

    return NULL;
}