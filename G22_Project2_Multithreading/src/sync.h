/*
 * sync.h — File Lock Table (synchronization primitives)
 * Owner: Member 1 (VISHNU TEJA VASAM)
 *
 * Provides per-file reader-writer locks so multiple threads can
 * safely operate on the same file without data corruption.
 * -------------------------------------------------------------------
 * DO NOT MODIFY this file without coordinating with Member 1.
 */

#ifndef SYNC_H
#define SYNC_H

#include <pthread.h>

/* Initialize the lock table. Call once at program start. */
void lock_table_init(void);

/* Destroy the lock table. Call once at program shutdown. */
void lock_table_destroy(void);

/*
 * Acquire a READ lock for the given filepath.
 * Multiple readers can hold this simultaneously.
 * Returns 0 on success, -1 on error.
 */
int file_read_lock(const char *filepath);

/*
 * Acquire a WRITE (exclusive) lock for the given filepath.
 * Only one writer at a time; blocks readers too.
 * Returns 0 on success, -1 on error.
 */
int file_write_lock(const char *filepath);

/*
 * Release whatever lock is held for the given filepath.
 * Returns 0 on success, -1 on error.
 */
int file_unlock(const char *filepath);

#endif /* SYNC_H */
