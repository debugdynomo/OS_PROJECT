/*
 * sync.c — File Lock Table implementation
 * Owner: Member 1 (VISHNU TEJA VASAM)
 *
 * A hash-table of per-file pthread_rwlock_t entries.
 * The table itself is protected by a global mutex.
 * -------------------------------------------------------------------
 * DO NOT MODIFY this file without coordinating with Member 1.
 */

#include "sync.h"
#include "common.h"

/* ── Internal data structures ──────────────────────────────────── */

#define LOCK_TABLE_SIZE 128

typedef struct lock_entry {
    char                filepath[MAX_PATH_LEN];
    pthread_rwlock_t    rwlock;
    int                 ref_count;     /* how many threads reference this */
    int                 in_use;        /* 1 = slot occupied, 0 = free     */
} lock_entry_t;

static lock_entry_t   lock_table[LOCK_TABLE_SIZE];
static pthread_mutex_t table_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ── Helper: find or create a lock entry ───────────────────────── */

/*
 * Returns the index of the entry for `filepath`.
 * If none exists, creates one.  Returns -1 if the table is full.
 * MUST be called while holding table_mutex.
 */
static int find_or_create(const char *filepath)
{
    int first_free = -1;

    for (int i = 0; i < LOCK_TABLE_SIZE; i++) {
        if (lock_table[i].in_use &&
            strcmp(lock_table[i].filepath, filepath) == 0) {
            lock_table[i].ref_count++;
            return i;
        }
        if (!lock_table[i].in_use && first_free == -1) {
            first_free = i;
        }
    }

    /* Not found — create a new entry */
    if (first_free == -1) {
        fprintf(stderr, "[sync] ERROR: lock table full (%d entries)\n",
                LOCK_TABLE_SIZE);
        return -1;
    }

    strncpy(lock_table[first_free].filepath, filepath, MAX_PATH_LEN - 1);
    lock_table[first_free].filepath[MAX_PATH_LEN - 1] = '\0';
    pthread_rwlock_init(&lock_table[first_free].rwlock, NULL);
    lock_table[first_free].ref_count = 1;
    lock_table[first_free].in_use    = 1;

    return first_free;
}

/* ── Helper: find an existing entry ────────────────────────────── */

static int find_entry(const char *filepath)
{
    for (int i = 0; i < LOCK_TABLE_SIZE; i++) {
        if (lock_table[i].in_use &&
            strcmp(lock_table[i].filepath, filepath) == 0) {
            return i;
        }
    }
    return -1;
}

/* ── Public API ────────────────────────────────────────────────── */

void lock_table_init(void)
{
    pthread_mutex_lock(&table_mutex);
    for (int i = 0; i < LOCK_TABLE_SIZE; i++) {
        lock_table[i].in_use    = 0;
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
            lock_table[i].in_use    = 0;
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

    int rc = pthread_rwlock_rdlock(&lock_table[idx].rwlock);
    if (rc != 0) {
        fprintf(stderr, "[sync] rdlock failed for '%s': %s\n",
                filepath, strerror(rc));
        return -1;
    }
    return 0;
}

int file_write_lock(const char *filepath)
{
    pthread_mutex_lock(&table_mutex);
    int idx = find_or_create(filepath);
    pthread_mutex_unlock(&table_mutex);

    if (idx < 0) return -1;

    int rc = pthread_rwlock_wrlock(&lock_table[idx].rwlock);
    if (rc != 0) {
        fprintf(stderr, "[sync] wrlock failed for '%s': %s\n",
                filepath, strerror(rc));
        return -1;
    }
    return 0;
}

int file_unlock(const char *filepath)
{
    pthread_mutex_lock(&table_mutex);
    int idx = find_entry(filepath);
    if (idx < 0) {
        pthread_mutex_unlock(&table_mutex);
        fprintf(stderr, "[sync] unlock: no lock found for '%s'\n", filepath);
        return -1;
    }

    int rc = pthread_rwlock_unlock(&lock_table[idx].rwlock);
    if (rc != 0) {
        pthread_mutex_unlock(&table_mutex);
        fprintf(stderr, "[sync] unlock failed for '%s': %s\n",
                filepath, strerror(rc));
        return -1;
    }

    lock_table[idx].ref_count--;
    if (lock_table[idx].ref_count <= 0) {
        pthread_rwlock_destroy(&lock_table[idx].rwlock);
        lock_table[idx].in_use    = 0;
        lock_table[idx].ref_count = 0;
    }

    pthread_mutex_unlock(&table_mutex);
    return 0;
}
