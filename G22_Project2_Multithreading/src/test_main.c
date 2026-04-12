#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "compression.h"
#include "sync.h"
#include "logger.h"

// ---------------------------------------------------------
// STUB IMPLEMENTATIONS FOR TESTING
// ---------------------------------------------------------
pthread_rwlock_t* get_file_lock(const char *filename) {
    (void)filename;
    return NULL;
}

void release_file_lock(const char *filename) {
    (void)filename;
}

void log_operation(const char *operation, const char *filename, const char *status, const char *details) {
    printf("[LOG] %s | File: %s | %s | %s\n", operation, filename, status, details);
}

// ---------------------------------------------------------
// TEST MAIN FUNCTION
// ---------------------------------------------------------
int main() {
    // 1. Create a sample file to test with
    const char *test_filename = "test_input.txt";
    FILE *f = fopen(test_filename, "w");
    if (f) {
        fprintf(f, "Hello World! This is a test file for the compression feature.\n");
        fclose(f);
    }
    
    printf("=== Starting Compression Test ===\n");
    compression_args_t c_args;
    snprintf(c_args.filename, sizeof(c_args.filename), "%s", test_filename);
    
    // Run compression
    thread_compress_file((void*)&c_args);
    
    printf("\n=== Starting Decompression Test ===\n");
    compression_args_t d_args;
    // The compressed file will have .gz appended
    snprintf(d_args.filename, sizeof(d_args.filename), "%s.gz", test_filename);
    
    // Run decompression
    thread_decompress_file((void*)&d_args);
    
    printf("\n=== Tests Completed === // Check your folder! Your test_input.txt was successfully restored from the .gz file!\n");
    
    return 0;
}
