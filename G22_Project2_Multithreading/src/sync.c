#include "sync.h"
#include "common.h"

#define LOCK_TABLE_SIZE 128

typedef struct lock_entry {
    char                filepath[MAX_PATH_LEN];
    pthread_rwlock_t    rwlock;
    int                 ref_count;
    int                 in_use;
} lock_entry_t;

static lock_entry_t   lock_table[LOCK_TABLE_SIZE];
static pthread_mutex_t table_mutex = PTHREAD_MUTEX_INITIALIZER;

static int find_or_create(const char *filepath)
{
    int first_free = -1;

    for (int i = 0; i < LOCK_TABLE_SIZE; i++) {
        if (lock_table[i].in_use && strcmp(lock_table[i].filepath, filepath) == 0) {
            lock_table[i].ref_count++;
            return i;
        }
        if (!lock_table[i].in_use && first_free == -1) {
            first_free = i;
        }
    }

    if (first_free == -1) {
        fprintf(stderr, "Error: lock table full\n");
        return -1;
    }

    strncpy(lock_table[first_free].filepath, filepath, MAX_PATH_LEN - 1);
    lock_table[first_free].filepath[MAX_PATH_LEN - 1] = '\0';
    pthread_rwlock_init(&lock_table[first_free].rwlock, NULL);
    lock_table[first_free].ref_count = 1;
    lock_table[first_free].in_use    = 1;

    return first_free;
}

static int find_entry(const char *filepath)
{
    for (int i = 0; i < LOCK_TABLE_SIZE; i++) {
        if (lock_table[i].in_use && strcmp(lock_table[i].filepath, filepath) == 0) {
            return i;
        }
    }
    return -1;
}

void lock_table_init(void)
{
    pthread_mutex_lock(&table_mutex);
    for (int i = 0; i < LOCK_TABLE_SIZE; i++) {
        lock_table[i].in_use    = 0;
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static FileLock lock_table[MAX_LOCKS];
static pthread_mutex_t table_mutex = PTHREAD_MUTEX_INITIALIZER;

void lock_table_init(void) {
    pthread_mutex_lock(&table_mutex);
    for (int i = 0; i < MAX_LOCKS; i++) {
        lock_table[i].in_use = 0;
        lock_table[i].ref_count = 0;
    }
    pthread_mutex_unlock(&table_mutex);
}

void lock_table_destroy(void)
{
    pthread_mutex_lock(&table_mutex);
    for (int i = 0; i < LOCK_TABLE_SIZE; i++) {
        if (lock_table[i].in_use) {
            pthread_rwlock_destroy(&lock_table[i].rwlock);
            lock_table[i].in_use = 0;
            lock_table[i].ref_count = 0;
        }
    }
    pthread_mutex_unlock(&table_mutex);
}

int file_read_lock(const char *filepath)
{
    pthread_mutex_lock(&table_mutex);
    int idx = find_or_create(filepath);
    pthread_mutex_unlock(&table_mutex);

    if (idx < 0) return -1;

    if (pthread_rwlock_rdlock(&lock_table[idx].rwlock) != 0) return -1;
    return 0;
}

int file_write_lock(const char *filepath)
{
    pthread_mutex_lock(&table_mutex);
    int idx = find_or_create(filepath);
    pthread_mutex_unlock(&table_mutex);

    if (idx < 0) return -1;

    if (pthread_rwlock_wrlock(&lock_table[idx].rwlock) != 0) return -1;
    return 0;
}

int file_unlock(const char *filepath)
{
    pthread_mutex_lock(&table_mutex);
    int idx = find_entry(filepath);
    if (idx < 0) {
        pthread_mutex_unlock(&table_mutex);
        return -1;
    }

    if (pthread_rwlock_unlock(&lock_table[idx].rwlock) != 0) {
        pthread_mutex_unlock(&table_mutex);
        return -1;
    }

    lock_table[idx].ref_count--;
    if (lock_table[idx].ref_count <= 0) {
        pthread_rwlock_destroy(&lock_table[idx].rwlock);
        lock_table[idx].in_use = 0;
        lock_table[idx].ref_count = 0;
    }

    pthread_mutex_unlock(&table_mutex);
    return 0;
}
void lock_table_destroy(void) {
    pthread_mutex_lock(&table_mutex);
    for (int i = 0; i < MAX_LOCKS; i++) {
        if (lock_table[i].in_use) {
            pthread_rwlock_destroy(&lock_table[i].rwlock);
            lock_table[i].in_use = 0;
        }
    }
    pthread_mutex_unlock(&table_mutex);
    pthread_mutex_destroy(&table_mutex);
}

static int get_or_create_lock(const char *filename) {
    int free_slot = -1;

    for (int i = 0; i < MAX_LOCKS; i++) {
        if (lock_table[i].in_use &&
            strncmp(lock_table[i].filename, filename, MAX_FILENAME_LEN) == 0) {
            lock_table[i].ref_count++;
            return i;
        }
        if (!lock_table[i].in_use && free_slot == -1)
            free_slot = i;
    }

    if (free_slot == -1) return -1;

    strncpy(lock_table[free_slot].filename, filename, MAX_FILENAME_LEN - 1);
    lock_table[free_slot].filename[MAX_FILENAME_LEN - 1] = '\0';

    pthread_rwlock_init(&lock_table[free_slot].rwlock, NULL);
    lock_table[free_slot].ref_count = 1;
    lock_table[free_slot].in_use = 1;

    return free_slot;
}

static void release_lock_entry(const char *filename) {
    for (int i = 0; i < MAX_LOCKS; i++) {
        if (lock_table[i].in_use &&
            strncmp(lock_table[i].filename, filename, MAX_FILENAME_LEN) == 0) {

            lock_table[i].ref_count--;

            if (lock_table[i].ref_count <= 0) {
                pthread_rwlock_destroy(&lock_table[i].rwlock);
                lock_table[i].in_use = 0;
            }
            return;
        }
    }
}

void file_read_lock(const char *filename) {
    pthread_mutex_lock(&table_mutex);

    int idx = get_or_create_lock(filename);

    pthread_mutex_unlock(&table_mutex);

    if (idx < 0) {
        fprintf(stderr, "Lock table full\n");
        return;
    }

    pthread_rwlock_rdlock(&lock_table[idx].rwlock);
}

void file_write_lock(const char *filename) {
    pthread_mutex_lock(&table_mutex);

    int idx = get_or_create_lock(filename);

    pthread_mutex_unlock(&table_mutex);

    if (idx < 0) {
        fprintf(stderr, "Lock table full\n");
        return;
    }

    pthread_rwlock_wrlock(&lock_table[idx].rwlock);
}

void file_unlock(const char *filename) {
    pthread_rwlock_t *lock = NULL;

    pthread_mutex_lock(&table_mutex);

    for (int i = 0; i < MAX_LOCKS; i++) {
        if (lock_table[i].in_use &&
            strncmp(lock_table[i].filename, filename, MAX_FILENAME_LEN) == 0) {

            lock = &lock_table[i].rwlock;
            release_lock_entry(filename);
            break;
        }
    }

    pthread_mutex_unlock(&table_mutex);

    if (lock)
        pthread_rwlock_unlock(lock);
}

void file_write_lock_two(const char *file1, const char *file2) {
    const char *first  = (strcmp(file1, file2) < 0) ? file1 : file2;
    const char *second = (strcmp(file1, file2) < 0) ? file2 : file1;

    file_write_lock(first);
    file_write_lock(second);
}

void file_unlock_two(const char *file1, const char *file2) {
    file_unlock(file1);
    file_unlock(file2);
}
