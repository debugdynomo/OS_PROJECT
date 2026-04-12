# Multithreaded File Management System
## Member 6: Compression & Decompression (Feature 9)

### 1. Feature Description
Feature 9 introduces file compression and decompression functionality using the `zlib` library. It allows users to compress text files into gzip (`.gz`) archives and later decompress them back to their original state. This feature is integrated directly into our multithreaded architecture, ensuring that heavy I/O operations from zlib do not interfere with other active file operations.

### 2. Implementation Overview
Two core functions are implemented inside `compression.c`:
*   `thread_compress_file(void *arg)`
*   `thread_decompress_file(void *arg)`

These act as thread routines that expect a `compression_args_t` structure containing the target filename. Both functions hook directly into the system's global error handling and logging system by calling `log_operation()` at the start, success, and error outcomes of the function.

### 3. Synchronization Design Rationale
To prevent race conditions and uncoordinated data corruption, rigorous Lock acquisitions are executed strictly before `zlib` parsing:
*   **Compression (`thread_compress_file`)**: 
    1.  Acquires a `pthread_rwlock_rdlock` (Read Lock) on the source file so concurrent reads from other system peers aren't blocked while the data is being piped.
    2.  Acquires a `pthread_rwlock_wrlock` (Exclusive Write Lock) on the newly created `.gz` file.
*   **Decompression (`thread_decompress_file`)**:
    1.  Acquires a `pthread_rwlock_rdlock` (Read Lock) on the compressed `.gz` source file map.
    2.  Acquires a `pthread_rwlock_wrlock` (Exclusive Write Lock) on the output file where data is restored.
*   **Safety**: All locks execute in a guaranteed linear sequence and are successfully cleaned up in reverse order regardless of if `gzread` or `gzwrite` processes throw an I/O error or close gracefully.

### 4. Build and Run Instructions
To properly compile the system, the compression module relies heavily on the `-lz` flag from the `zlib1g-dev` dependency.

```bash
# Compilation
gcc -c src/compression.c -Wall -Wextra

# Final Build Command (Must include -lz to expose gzopen/gzread/gzwrite)
gcc main.o sync.o logger.o file_ops.o compression.o -o MultithreadedFileSys -lpthread -lz
```

### 5. Open Questions Answered
*   **Compression Library**: As decided, `zlib` was selected per the POSIX standard as it's structurally safer and perfectly integrates with standard POSIX threads compared to custom approaches like RLE. 
*   **Build Environment**: As zlib and `pthreads` are fully functional POSIX systems, MSYS2/MinGW or Linux Subsystems (WSL/Ubuntu) are strictly required for compiling and linking `-lpthread -lz`.
