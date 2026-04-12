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
