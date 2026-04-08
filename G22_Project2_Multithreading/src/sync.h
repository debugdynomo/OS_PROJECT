#ifndef SYNC_H
#define SYNC_H

#include <pthread.h>

void lock_table_init(void);
void lock_table_destroy(void);

int file_read_lock(const char *filepath);
int file_write_lock(const char *filepath);
int file_unlock(const char *filepath);

#endif
