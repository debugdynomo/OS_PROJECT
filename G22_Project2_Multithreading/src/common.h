/*
 * common.h — Shared definitions for Multithreaded File Manager
 * Owner: Member 1 (VISHNU TEJA VASAM)
 *
 * Contains argument structs and shared constants used by all modules.
 * -------------------------------------------------------------------
 * DO NOT MODIFY this file without coordinating with Member 1.
 */

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

/* ── Constants ─────────────────────────────────────────────────── */

#define MAX_PATH_LEN     512
#define MAX_THREADS      64
#define BUFFER_SIZE      4096
#define LOG_FILE_PATH    "logs/operations.log"

/* ── Thread argument structs ───────────────────────────────────── */

/* Used by: thread_read_file, thread_write_file, thread_delete_file,
 *          thread_compress_file, thread_decompress_file,
 *          thread_display_metadata
 */
typedef struct {
    char filepath[MAX_PATH_LEN];
} file_arg_t;

/* Used by: thread_write_file (when content is provided) */
typedef struct {
    char filepath[MAX_PATH_LEN];
    char content[BUFFER_SIZE];
    int  append;   /* 1 = append, 0 = overwrite */
} write_arg_t;

/* Used by: thread_rename_file */
typedef struct {
    char old_path[MAX_PATH_LEN];
    char new_path[MAX_PATH_LEN];
} rename_arg_t;

/* Used by: thread_copy_file */
typedef struct {
    char src_path[MAX_PATH_LEN];
    char dst_path[MAX_PATH_LEN];
} copy_arg_t;

/* ── Global shutdown flag (set by signal handler) ──────────────── */

extern volatile sig_atomic_t g_shutdown;

#endif /* COMMON_H */
