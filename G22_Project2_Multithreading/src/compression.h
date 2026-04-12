#ifndef COMPRESSION_H
#define COMPRESSION_H

void *thread_compress_file(void *arg);
void *thread_decompress_file(void *arg);

#endif
#include <pthread.h>

typedef struct {
    char filename[256];
} compression_args_t;

/**
 * @brief Thread function to compress a file using zlib.
 * 
 * Acquires a read lock on the input file and a write lock on the 
 * output (.gz) file. Logs operations via logger.c.
 * 
 * @param arg Pointer to a compression_args_t structure
 * @return void* Always returns NULL
 */
void* thread_compress_file(void *arg);

/**
 * @brief Thread function to decompress a file using zlib.
 * 
 * Acquires a read lock on the input (.gz) file and a write lock on the 
 * output file. Logs operations via logger.c.
 * 
 * @param arg Pointer to a compression_args_t structure
 * @return void* Always returns NULL
 */
void* thread_decompress_file(void *arg);

#endif // COMPRESSION_H
