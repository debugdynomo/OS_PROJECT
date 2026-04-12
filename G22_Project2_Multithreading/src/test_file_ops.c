#include "file_ops.h"
#include "sync.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

/* ---------- Create sample file ---------- */
void create_sample_file(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        exit(1);
    }

    fprintf(fp, "Line 1: Hello World\n");
    fprintf(fp, "Line 2: Multithreading Test\n");
    fprintf(fp, "Line 3: OS Project\n");

    fclose(fp);
}

/* ---------- Test Concurrent Reads ---------- */
void test_concurrent_reads(const char *filename) {
    printf("\n=== TEST: Concurrent Reads ===\n");

    int n = 3;
    pthread_t threads[3];
    ReadArgs args[3];

    for (int i = 0; i < n; i++) {
        args[i].filename = filename;
        args[i].thread_id = i + 1;

        pthread_create(&threads[i], NULL,
                       thread_read_file, &args[i]);
    }

    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("=== Done: Concurrent Reads ===\n");
}

/* ---------- Test Exclusive Writes ---------- */
void test_exclusive_writes(const char *filename) {
    printf("\n=== TEST: Exclusive Writes ===\n");

    pthread_t threads[3];
    WriteArgs args[3];

    const char *messages[] = {
        "Writer 1 data",
        "Writer 2 data",
        "Writer 3 data"
    };

    for (int i = 0; i < 3; i++) {
        args[i].filename = filename;
        args[i].content = messages[i];
        args[i].mode = WRITE_MODE_APPEND;
        args[i].thread_id = i + 1;

        pthread_create(&threads[i], NULL,
                       thread_write_file, &args[i]);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("=== Done: Exclusive Writes ===\n");
}

/* ---------- Main ---------- */
int main() {
    const char *filename = "test_file.txt";

    lock_table_init();

    create_sample_file(filename);

    test_concurrent_reads(filename);

    test_exclusive_writes(filename);

    lock_table_destroy();

    return 0;
}