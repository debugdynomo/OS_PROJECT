#ifndef FILE_OPS_H
#define FILE_OPS_H


void *thread_read_file(void *arg);
void *thread_write_file(void *arg);
void *thread_delete_file(void *arg);
void *thread_rename_file(void *arg);
void *thread_copy_file(void *arg);
void *thread_display_metadata(void *arg);

#endif

#include <stddef.h>

typedef struct {
    const char *filename;
    int thread_id;
}ReadArgs;

typedef enum {
    WRITE_MODE_APPEND,
    WRITE_MODE_OVERWRITE
}WriteMode;

typedef struct {
    const char *filename;
    const char *content;
    WriteMode mode;
    int thread_id;
}WriteArgs;

void *thread_read_file(void *arg);
void *thread_write_file(void *arg);

void demo_concurrent_reads(const char *filename, int count);

#endif

#ifndef FILE_OPS_H
#define FILE_OPS_H

#include <pthread.h>

// Struct for delete
typedef struct {
    char filename[256];
} file_args;

// Struct for rename
typedef struct {
    char oldname[256];
    char newname[256];
} rename_args;

// Function declarations
void* thread_delete_file(void* arg);
void* thread_rename_file(void* arg);

#endif
