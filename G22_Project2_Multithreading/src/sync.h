#ifndef SYNC_H
#define SYNC_H

#include <pthread.h>

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