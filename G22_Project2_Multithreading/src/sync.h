#ifndef SYNC_H
#define SYNC_H

#include <pthread.h>

void lock_table_init(void);
void lock_table_destroy(void);

int file_read_lock(const char *filepath);
int file_write_lock(const char *filepath);
int file_unlock(const char *filepath);

#endif
#define MAX_LOCKS 128
#define MAX_FILENAME_LEN 256

typedef struct {
    char filename[MAX_FILENAME_LEN];  
    pthread_rwlock_t rwlock;         
    int ref_count;                    
    int in_use;                       
} FileLock;

void lock_table_init(void);
void lock_table_destroy(void);

void file_read_lock(const char *filename);
void file_write_lock(const char *filename);
void file_unlock(const char *filename);

void file_write_lock_two(const char *file1, const char *file2);
void file_unlock_two(const char *file1, const char *file2);

#endif 
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
