#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_PATH_LEN     512
#define MAX_THREADS      64
#define BUFFER_SIZE      4096
#define LOG_FILE_PATH    "logs/operations.log"

typedef struct {
    char filepath[MAX_PATH_LEN];
} file_arg_t;

typedef struct {
    char filepath[MAX_PATH_LEN];
    char content[BUFFER_SIZE];
    int  append;
} write_arg_t;

typedef struct {
    char old_path[MAX_PATH_LEN];
    char new_path[MAX_PATH_LEN];
} rename_arg_t;

typedef struct {
    char src_path[MAX_PATH_LEN];
    char dst_path[MAX_PATH_LEN];
} copy_arg_t;

extern volatile sig_atomic_t g_shutdown;

#endif
