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