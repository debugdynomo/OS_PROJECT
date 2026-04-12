#ifndef SYNC_H
#define SYNC_H

#include <pthread.h>

/**
 * @brief Get the file lock object for a specific filename.
 * 
 * Returns a pointer to the rwlock for the file 
 * (creates one if it doesn't exist).
 * 
 * @param filename Name of the file.
 * @return pthread_rwlock_t* Pointer to the rwlock.
 */
pthread_rwlock_t* get_file_lock(const char *filename);

/**
 * @brief Release the file lock reference.
 * 
 * Decrements the reference count and cleans up the lock 
 * if it's no longer needed.
 * 
 * @param filename Name of the file.
 */
void release_file_lock(const char *filename);

#endif // SYNC_H
