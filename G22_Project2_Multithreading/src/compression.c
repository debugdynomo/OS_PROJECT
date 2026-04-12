#include "compression.h"
#include "common.h"
#include "sync.h"
#include "logger.h"
#include <zlib.h>

void *thread_compress_file(void *arg)
{
    file_arg_t *fa = (file_arg_t *)arg;

    log_operation("COMPRESS", fa->filepath, "START", NULL);

    if (file_read_lock(fa->filepath) != 0) {
        log_operation("COMPRESS", fa->filepath, "FAILURE", "Could not acquire lock");
        free(fa);
        return NULL;
    }

    char out_path[MAX_PATH_LEN];
    snprintf(out_path, sizeof(out_path), "%s.gz", fa->filepath);

    FILE *src = fopen(fa->filepath, "rb");
    if (!src) {
        log_operation("COMPRESS", fa->filepath, "FAILURE", strerror(errno));
        file_unlock(fa->filepath);
        free(fa);
        return NULL;
    }

    gzFile gz = gzopen(out_path, "wb");
    if (!gz) {
        log_operation("COMPRESS", out_path, "FAILURE", "gzopen failed");
        fclose(src);
        file_unlock(fa->filepath);
        free(fa);
        return NULL;
    }

    char buf[BUFFER_SIZE];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (gzwrite(gz, buf, (unsigned)n) == 0) {
            log_operation("COMPRESS", out_path, "FAILURE", "gzwrite failed");
            gzclose(gz);
            fclose(src);
            file_unlock(fa->filepath);
            free(fa);
            return NULL;
        }
    }

    fclose(src);
    gzclose(gz);

    file_unlock(fa->filepath);

    char detail[MAX_PATH_LEN + 32];
    snprintf(detail, sizeof(detail), "output: %s", out_path);
    log_operation("COMPRESS", fa->filepath, "SUCCESS", detail);

    free(fa);
    return NULL;
}

void *thread_decompress_file(void *arg)
{
    file_arg_t *fa = (file_arg_t *)arg;

    log_operation("DECOMPRESS", fa->filepath, "START", NULL);

    if (file_read_lock(fa->filepath) != 0) {
        log_operation("DECOMPRESS", fa->filepath, "FAILURE", "Could not acquire lock");
        free(fa);
        return NULL;
    }

    char out_path[MAX_PATH_LEN];
    strncpy(out_path, fa->filepath, MAX_PATH_LEN - 1);
    out_path[MAX_PATH_LEN - 1] = '\0';

    size_t len = strlen(out_path);
    if (len > 3 && strcmp(out_path + len - 3, ".gz") == 0) {
        out_path[len - 3] = '\0';
    } else {
        snprintf(out_path, sizeof(out_path), "%s.out", fa->filepath);
    }

    gzFile gz = gzopen(fa->filepath, "rb");
    if (!gz) {
        log_operation("DECOMPRESS", fa->filepath, "FAILURE", "gzopen failed");
        file_unlock(fa->filepath);
        free(fa);
        return NULL;
    }

    FILE *dst = fopen(out_path, "wb");
    if (!dst) {
        log_operation("DECOMPRESS", out_path, "FAILURE", strerror(errno));
        gzclose(gz);
        file_unlock(fa->filepath);
        free(fa);
        return NULL;
    }

    char buf[BUFFER_SIZE];
    int bytes;
    while ((bytes = gzread(gz, buf, sizeof(buf))) > 0) {
        if (fwrite(buf, 1, (size_t)bytes, dst) != (size_t)bytes) {
            log_operation("DECOMPRESS", out_path, "FAILURE", "Write error");
            fclose(dst);
            gzclose(gz);
            file_unlock(fa->filepath);
            free(fa);
            return NULL;
        }
    }

    fclose(dst);
    gzclose(gz);

    file_unlock(fa->filepath);

    char detail[MAX_PATH_LEN + 32];
    snprintf(detail, sizeof(detail), "output: %s", out_path);
    log_operation("DECOMPRESS", fa->filepath, "SUCCESS", detail);

    free(fa);
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <zlib.h>
#include "compression.h"
#include "sync.h"
#include "logger.h"

#define CHUNK_SIZE 8192

void* thread_compress_file(void *arg) {
    if (!arg) return NULL;
    
    compression_args_t *args = (compression_args_t *)arg;
    char out_filename[512];
    snprintf(out_filename, sizeof(out_filename), "%s.gz", args->filename);
    
    log_operation("COMPRESS_START", args->filename, "INFO", "Started compression");
    
    // Acquire read lock for source file
    pthread_rwlock_t *src_lock = get_file_lock(args->filename);
    if (src_lock) {
        pthread_rwlock_rdlock(src_lock);
    }
    
    // Acquire write lock for destination file
    pthread_rwlock_t *dest_lock = get_file_lock(out_filename);
    if (dest_lock) {
        pthread_rwlock_wrlock(dest_lock);
    }
    
    FILE *src = fopen(args->filename, "rb");
    if (!src) {
        log_operation("COMPRESS_ERROR", args->filename, "ERROR", "Failed to open source file");
        
        if (dest_lock) {
            pthread_rwlock_unlock(dest_lock);
            release_file_lock(out_filename);
        }
        if (src_lock) {
            pthread_rwlock_unlock(src_lock);
            release_file_lock(args->filename);
        }
        return NULL;
    }
    
    gzFile dest = gzopen(out_filename, "wb");
    if (!dest) {
        log_operation("COMPRESS_ERROR", out_filename, "ERROR", "Failed to open destination gz file");
        fclose(src);
        
        if (dest_lock) {
            pthread_rwlock_unlock(dest_lock);
            release_file_lock(out_filename);
        }
        if (src_lock) {
            pthread_rwlock_unlock(src_lock);
            release_file_lock(args->filename);
        }
        return NULL;
    }
    char buffer[CHUNK_SIZE];
    size_t bytes_read;
    int success = 1;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        int bytes_written = gzwrite(dest, buffer, bytes_read);
        if (bytes_written == 0) {
            success = 0;
            break;
        }
    }
    
    fclose(src);
    gzclose(dest);
    
    if (success) {
        log_operation("COMPRESS_SUCCESS", args->filename, "SUCCESS", "File compressed successfully into .gz");
    } else {
        log_operation("COMPRESS_ERROR", args->filename, "ERROR", "Error writing to .gz file during compression");
    }
    
    if (dest_lock) {
        pthread_rwlock_unlock(dest_lock);
        release_file_lock(out_filename);
    }
    if (src_lock) {
        pthread_rwlock_unlock(src_lock);
        release_file_lock(args->filename);
    }
    
    return NULL;
}

void* thread_decompress_file(void *arg) {
    if (!arg) return NULL;
    
    compression_args_t *args = (compression_args_t *)arg;
    char out_filename[512];
    
    size_t len = strlen(args->filename);
    if (len > 3 && strcmp(args->filename + len - 3, ".gz") == 0) {
        snprintf(out_filename, len - 2, "%s", args->filename);
    } else {
        snprintf(out_filename, sizeof(out_filename), "%s.uncompressed", args->filename);
    }
    
    log_operation("DECOMPRESS_START", args->filename, "INFO", "Started decompression");
    
    // Acquire read lock for source file
    pthread_rwlock_t *src_lock = get_file_lock(args->filename);
    if (src_lock) {
        pthread_rwlock_rdlock(src_lock);
    }
    
    // Acquire write lock for destination file
    pthread_rwlock_t *dest_lock = get_file_lock(out_filename);
    if (dest_lock) {
        pthread_rwlock_wrlock(dest_lock);
    }
    
    gzFile src = gzopen(args->filename, "rb");
    if (!src) {
        log_operation("DECOMPRESS_ERROR", args->filename, "ERROR", "Failed to open source gz file");
        
        if (dest_lock) {
            pthread_rwlock_unlock(dest_lock);
            release_file_lock(out_filename);
        }
        if (src_lock) {
            pthread_rwlock_unlock(src_lock);
            release_file_lock(args->filename);
        }
        return NULL;
    }
    
    FILE *dest = fopen(out_filename, "wb");
    if (!dest) {
        log_operation("DECOMPRESS_ERROR", out_filename, "ERROR", "Failed to open destination file");
        gzclose(src);
        
        if (dest_lock) {
            pthread_rwlock_unlock(dest_lock);
            release_file_lock(out_filename);
        }
        if (src_lock) {
            pthread_rwlock_unlock(src_lock);
            release_file_lock(args->filename);
        }
        return NULL;
    }
    
    char buffer[CHUNK_SIZE];
    int bytes_read;
    int success = 1;
    
    while ((bytes_read = gzread(src, buffer, sizeof(buffer))) > 0) {
        size_t bytes_written = fwrite(buffer, 1, bytes_read, dest);
        if (bytes_written != (size_t)bytes_read) {
            success = 0;
            break;
        }
    }
    
    if (bytes_read < 0) {
        success = 0; 
    }
    
    gzclose(src);
    fclose(dest);
    
    if (success) {
        log_operation("DECOMPRESS_SUCCESS", args->filename, "SUCCESS", "File decompressed successfully");
    } else {
        log_operation("DECOMPRESS_ERROR", args->filename, "ERROR", "Error during decompression");
    }
    
    if (dest_lock) {
        pthread_rwlock_unlock(dest_lock);
        release_file_lock(out_filename);
    }
    if (src_lock) {
        pthread_rwlock_unlock(src_lock);
        release_file_lock(args->filename);
    }
    
    return NULL;
}
