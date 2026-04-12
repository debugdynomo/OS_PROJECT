# G22 — Multithreaded File Management System

## Project Overview

This project implements a **multithreaded file management system** in C using POSIX threads (`pthreads`). It is a standalone Linux/WSL program that provides an interactive CLI for performing common file operations — each dispatched in its own thread with proper synchronization, error handling, and logging.

### Key Highlights

- **9 core features**: Read, Write, Delete, Rename, Copy, Metadata, Compress, Decompress, Logging
- **Thread-safe**: Per-file reader–writer locks prevent data races and corruption
- **Deadlock-free**: Lock ordering (alphabetical) prevents deadlock during rename
- **Graceful shutdown**: `SIGINT` handler ensures all threads finish before exit
- **zlib compression**: Industry-standard gzip compression/decompression

---

## Team Members & Individual Contributions

| # | Member | Role | Files Owned | Features |
|---|--------|------|-------------|----------|
| 1 | **Vishnu** | Core Architecture & Synchronization | `main.c`, `sync.c`, `sync.h`, `signals.c`, `signals.h`, `Makefile`, `common.h` | Foundation, CLI menu, thread dispatching, signal handling |
| 2 | **Pratik** | Concurrent Read & Exclusive Write | `read_write.c` | Feature 1 — Concurrent File Reading, Feature 2 — Exclusive File Writing |
| 3 | **Akshara** | File Deletion & Renaming | `delete_rename.c` | Feature 3 — File Deletion, Feature 4 — File Renaming |
| 4 | **Vishesh** | File Copying & Metadata Display | `copy_metadata.c` | Feature 5 — File Copying, Feature 6 — File Metadata Display |
| 5 | **Saiteja** | Error Handling & Logging | `logger.c`, `logger.h` | Feature 7 — Error Handling, Feature 8 — Logging Operations |
| 6 | **Purna** | Compression & Decompression | `compression.c`, `compression.h` | Feature 9 — Compression and Decompression |

---

## Detailed Member Contributions

### Member 1 — Vishnu (Core Architecture & Synchronization)

**Files**: `main.c`, `sync.c`, `sync.h`, `signals.c`, `signals.h`, `common.h`, `Makefile`

Vishnu designed and implemented the **core architecture** of the system:

- **`main.c`** — The interactive CLI menu loop. Accepts user choices (1–10), collects input arguments (file paths, content), and spawns a detached pthread for each operation using `pthread_create()`. Maintains an active-thread counter protected by a `pthread_mutex_t` and a `pthread_cond_t` for the shutdown wait. The `thread_wrapper()` function automatically decrements the counter when a thread completes.

- **`sync.c` / `sync.h`** — The **file lock table**, which is the heart of the synchronization system. Implements a fixed-size array (`LOCK_TABLE_SIZE = 128`) of `lock_entry_t` structs, each containing a filename, a `pthread_rwlock_t`, a reference count, and an in-use flag. A global `pthread_mutex_t` protects the table itself. Key functions:
  - `lock_table_init()` / `lock_table_destroy()` — lifecycle management
  - `file_read_lock(filepath)` — acquires a shared reader lock (creates entry if needed)
  - `file_write_lock(filepath)` — acquires an exclusive writer lock
  - `file_unlock(filepath)` — releases the lock and destroys the entry when the ref count drops to zero

- **`signals.c` / `signals.h`** — Installs a `SIGINT` handler via `signal()` that sets a global `volatile sig_atomic_t g_shutdown` flag. The main loop checks this flag before each iteration, and the shutdown path calls `wait_all_threads()` to join all active threads before exit. Also ignores `SIGPIPE`.

- **`common.h`** — Shared definitions: `MAX_PATH_LEN` (512), `MAX_THREADS` (64), `BUFFER_SIZE` (4096), `LOG_FILE_PATH`, and the four argument structs (`file_arg_t`, `write_arg_t`, `rename_arg_t`, `copy_arg_t`) used to pass parameters to thread functions.

- **`Makefile`** — Build system with targets `all`, `run`, and `clean`. Compiles all source files from `src/` with flags `-Wall -Wextra -pthread -g` and links with `-lpthread -lz`.

---

### Member 2 — Pratik (Concurrent Read & Exclusive Write)

**File**: `read_write.c`

Pratik implemented the core file I/O operations:

- **`thread_read_file(void *arg)`** — **Feature 1: Concurrent File Reading**
  - Acquires a **shared read lock** (`file_read_lock`) on the target file
  - Opens the file with `fopen(path, "r")` and reads it line-by-line using `fgets()`
  - Prints the file contents to stdout between `--- Contents of ... ---` markers
  - Releases the lock via `file_unlock()` after completing the read
  - **Concurrency**: Multiple reader threads can read the same file simultaneously because `pthread_rwlock_rdlock` allows concurrent shared access

- **`thread_write_file(void *arg)`** — **Feature 2: Exclusive File Writing**
  - Acquires an **exclusive write lock** (`file_write_lock`) on the target file
  - Supports both **append** mode (`"a"`) and **overwrite** mode (`"w"`), determined by user input
  - Writes content using `fprintf()` and closes the file
  - Releases the write lock after completing the operation
  - **Exclusivity**: Only one writer thread can access the file at a time; all reader and writer threads are blocked until the write completes

Both functions log `START`, `SUCCESS`, or `FAILURE` status via `log_operation()` and free their argument struct before returning.

---

### Member 3 — Akshara (File Deletion & Renaming)

**File**: `delete_rename.c`

Akshara implemented safe file deletion and deadlock-free renaming:

- **`thread_delete_file(void *arg)`** — **Feature 3: File Deletion**
  - Acquires an **exclusive write lock** before deletion to prevent other threads from reading/writing the file during removal
  - Calls `remove(filepath)` to delete the file from the filesystem
  - Logs the operation and releases the lock

- **`thread_rename_file(void *arg)`** — **Feature 4: File Renaming**
  - Implements **deadlock-free two-lock acquisition** by always locking files in **alphabetical order** (`strcmp`-based comparison of old and new paths)
  - Acquires exclusive write locks on both the source and destination paths
  - Calls `rename(old_path, new_path)` to perform the rename
  - Releases both locks in reverse order on success or failure
  - **Deadlock prevention**: By enforcing a consistent lock ordering, two concurrent rename operations (e.g., A→B and B→A) cannot deadlock because both threads will always acquire locks in the same order

---

### Member 4 — Vishesh (File Copying & Metadata Display)

**File**: `copy_metadata.c`

Vishesh implemented file copying with careful lock management and metadata retrieval:

- **`thread_copy_file(void *arg)`** — **Feature 5: File Copying**
  - Acquires a **read lock** on the source file (allows other threads to read the source concurrently)
  - Opens the source in binary read mode (`"rb"`) and the destination in binary write mode (`"wb"`)
  - Copies data in `BUFFER_SIZE` (4096-byte) chunks using `fread()` / `fwrite()`
  - Validates that every `fwrite()` writes the expected number of bytes
  - Releases the source lock after the copy completes
  - The destination file is written independently (no lock needed for a new file)

- **`thread_display_metadata(void *arg)`** — **Feature 6: File Metadata Display**
  - Acquires a **read lock** for thread safety during the `stat()` call
  - Retrieves file metadata using the POSIX `stat()` system call
  - Displays: file size (bytes), permissions (octal), last accessed time, last modified time, and last status change time
  - Times are formatted using `ctime()` for human-readable output

---

### Member 5 — Saiteja (Error Handling & Logging)

**Files**: `logger.c`, `logger.h`

Saiteja implemented the thread-safe logging subsystem and error handling strategy:

- **`logger_init(const char *log_file_path)`** — **Feature 8: Logging Operations**
  - Opens the log file in append mode (`"a"`)
  - Protected by `pthread_mutex_t` to prevent race conditions during initialization
  - Returns 0 on success, -1 on failure

- **`log_operation(operation, filepath, status, detail)`** — Core logging function
  - **Thread-safe**: All log writes are serialized by a `pthread_mutex_t`
  - Generates timestamps using `strftime()` with format `[YYYY-MM-DD HH:MM:SS]`
  - Includes the calling thread's ID via `pthread_self()` for traceability
  - Output format: `[timestamp] [TID:xxxxx] [OPERATION] filepath — status — detail`
  - Writes to **both** the log file and `stderr` for real-time visibility
  - Calls `fflush()` after every write to ensure durability

- **`logger_close()`** — Flushes and closes the log file, then destroys the mutex

- **Feature 7: Error Handling Strategy**
  - Every thread function checks return values of all system calls (`fopen`, `fread`, `fwrite`, `stat`, `rename`, `remove`, lock functions)
  - On failure: logs the error with `strerror(errno)`, releases any held locks, frees allocated memory, and returns `NULL`
  - The main thread tracks active thread count for clean shutdown
  - System-wide error codes: functions return `0` for success, `-1` for failure

---

### Member 6 — Purna (Compression & Decompression)

**Files**: `compression.c`, `compression.h`

Purna implemented file compression and decompression using the zlib library:

- **`thread_compress_file(void *arg)`** — **Feature 9a: File Compression**
  - Acquires a **read lock** on the source file
  - Opens the source with `fopen(path, "rb")` and the output with `gzopen(path.gz, "wb")`
  - Reads the source in `BUFFER_SIZE` chunks and writes compressed data using `gzwrite()`
  - Produces a `.gz` output file (e.g., `sample.txt` → `sample.txt.gz`)
  - Logs the output file path on success

- **`thread_decompress_file(void *arg)`** — **Feature 9b: File Decompression**
  - Acquires a **read lock** on the `.gz` source file
  - Automatically determines the output filename by stripping the `.gz` extension (or appending `.out` if no `.gz` extension)
  - Opens the compressed file with `gzopen(path, "rb")` and writes decompressed data using `gzread()` + `fwrite()`
  - Validates write integrity by comparing bytes written with bytes read

Both functions handle errors at every step and release locks/close files properly on any failure path.

---

## Architecture

### System Overview

```
┌────────────────────────────────────────────────────────┐
│                     main.c                             │
│            Interactive CLI Menu (loop)                  │
│    ┌──────────────────────────────────────────────┐     │
│    │  User selects operation → spawn_thread()     │     │
│    │  thread_wrapper() auto-decrements counter    │     │
│    └──────────────────────────────────────────────┘     │
└─────────┬────────┬────────┬────────┬───────────────────┘
          │        │        │        │
          ▼        ▼        ▼        ▼
   ┌──────────┐ ┌─────────┐ ┌──────────┐ ┌──────────────┐
   │read_write│ │ delete_ │ │  copy_   │ │ compression  │
   │   .c     │ │rename.c │ │metadata.c│ │     .c       │
   │ (Pratik) │ │(Akshara)│ │(Vishesh) │ │  (Purna)     │
   └────┬─────┘ └───┬─────┘ └────┬─────┘ └──────┬───────┘
        │            │            │               │
        ▼            ▼            ▼               ▼
   ┌─────────────────────────────────────────────────────┐
   │                    sync.c (Vishnu)                  │
   │         File Lock Table — pthread_rwlock_t          │
   │    file_read_lock() / file_write_lock() / unlock()  │
   └─────────────────────────────────────────────────────┘
        │
        ▼
   ┌─────────────────────────────────────────────────────┐
   │                  logger.c (Saiteja)                 │
   │         Thread-safe logging — pthread_mutex_t       │
   │    log_operation() → logs/operations.log + stderr   │
   └─────────────────────────────────────────────────────┘
```

### Synchronization Strategy

| Operation | Lock Type | Rationale |
|-----------|-----------|-----------|
| Read | `pthread_rwlock_rdlock` | Multiple readers allowed simultaneously |
| Write | `pthread_rwlock_wrlock` | Exclusive access — blocks all readers and writers |
| Delete | `pthread_rwlock_wrlock` | Exclusive — modifies filesystem entry |
| Rename | `pthread_rwlock_wrlock` × 2 (ordered) | Exclusive on both paths; alphabetical ordering prevents deadlock |
| Copy | `pthread_rwlock_rdlock` on source | Source is read-only during copy; destination is a new file |
| Metadata | `pthread_rwlock_rdlock` | Read-only `stat()` operation |
| Compress | `pthread_rwlock_rdlock` | Source file is read-only |
| Decompress | `pthread_rwlock_rdlock` | Compressed source is read-only |
| Logging | `pthread_mutex_lock` | Serializes all log writes to a single file |

---

## File Structure

```
G22_Project2_Multithreading/
├── Makefile                        # Build system (Vishnu)
├── src/
│   ├── common.h                    # Shared types & constants (Vishnu)
│   ├── main.c                      # CLI menu & thread dispatcher (Vishnu)
│   ├── sync.c                      # File lock table implementation (Vishnu)
│   ├── sync.h                      # Lock table API (Vishnu)
│   ├── signals.c                   # SIGINT handler (Vishnu)
│   ├── signals.h                   # Signal handler API (Vishnu)
│   ├── read_write.c                # Read & Write operations (Pratik)
│   ├── file_ops.h                  # File operation prototypes (Shared)
│   ├── delete_rename.c             # Delete & Rename operations (Akshara)
│   ├── copy_metadata.c             # Copy & Metadata operations (Vishesh)
│   ├── logger.c                    # Thread-safe logger (Saiteja)
│   ├── logger.h                    # Logger API (Saiteja)
│   ├── compression.c               # Compress/Decompress with zlib (Purna)
│   └── compression.h               # Compression API (Purna)
├── docs/
│   └── documentation.md            # This file
├── test_files/                     # Sample test files
│   └── ...
└── logs/                           # Auto-created at runtime
    └── operations.log              # All logged operations
```

---

## Features Summary

### Feature 1 — Concurrent File Reading
- **Owner**: Pratik
- **Function**: `thread_read_file()` in `read_write.c`
- **Description**: Multiple threads can read the same file at the same time using shared reader locks (`pthread_rwlock_rdlock`). The file contents are printed to stdout.
- **Lock**: Shared read lock on the file

### Feature 2 — Exclusive File Writing
- **Owner**: Pratik
- **Function**: `thread_write_file()` in `read_write.c`
- **Description**: Only one thread can write to a file at a time. Supports both append and overwrite modes. The write lock blocks all other readers and writers.
- **Lock**: Exclusive write lock on the file

### Feature 3 — File Deletion
- **Owner**: Akshara
- **Function**: `thread_delete_file()` in `delete_rename.c`
- **Description**: Safely deletes a file by acquiring an exclusive write lock first, preventing any other thread from accessing the file during deletion. Uses `remove()`.
- **Lock**: Exclusive write lock on the file

### Feature 4 — File Renaming
- **Owner**: Akshara
- **Function**: `thread_rename_file()` in `delete_rename.c`
- **Description**: Renames a file using `rename()`. Acquires exclusive write locks on both the old and new paths. Locks are acquired in alphabetical order to prevent deadlock.
- **Lock**: Exclusive write locks on both old and new paths (ordered)

### Feature 5 — File Copying
- **Owner**: Vishesh
- **Function**: `thread_copy_file()` in `copy_metadata.c`
- **Description**: Copies a file byte-by-byte in 4096-byte chunks. Acquires a read lock on the source (allowing concurrent readers) and writes to the destination independently.
- **Lock**: Shared read lock on source file

### Feature 6 — File Metadata Display
- **Owner**: Vishesh
- **Function**: `thread_display_metadata()` in `copy_metadata.c`
- **Description**: Uses the POSIX `stat()` system call to display file size, permissions, last accessed time, last modified time, and last status change time.
- **Lock**: Shared read lock on the file

### Feature 7 — Error Handling
- **Owner**: Saiteja
- **Description**: Every function checks return values of all system calls. On failure, the error is logged with `strerror(errno)`, any held locks are released, allocated memory is freed, and the thread returns `NULL`. The main thread tracks active threads for clean shutdown.
- **Pattern**: Check → Log → Release → Free → Return

### Feature 8 — Logging Operations
- **Owner**: Saiteja
- **Function**: `log_operation()` in `logger.c`
- **Description**: Thread-safe logging using a `pthread_mutex_t`. Every operation logs timestamped entries with thread ID, operation type, file path, status, and details. Logs are written to both `logs/operations.log` and `stderr`.
- **Format**: `[2026-04-12 17:00:00] [TID:12345] [WRITE] test.txt — SUCCESS — mode=append`

### Feature 9 — Compression & Decompression
- **Owner**: Purna
- **Functions**: `thread_compress_file()`, `thread_decompress_file()` in `compression.c`
- **Description**: Uses zlib (`gzopen`, `gzwrite`, `gzread`) for gzip compression/decompression. Compress produces a `.gz` file; decompress strips the `.gz` extension to restore the original.
- **Lock**: Shared read lock on source file

---

## How to Build & Run

### Prerequisites

- **OS**: Linux or Windows Subsystem for Linux (WSL)
- **Compiler**: GCC with C11 support
- **Libraries**: POSIX threads (`libpthread`), zlib (`libz`)

Install zlib if not already available:
```bash
# Debian/Ubuntu
sudo apt-get install zlib1g-dev

# Fedora/RHEL
sudo dnf install zlib-devel
```

### Build

```bash
cd G22_Project2_Multithreading
make clean
make
```

Expected output:
```
  ✓ Build successful: ./file_manager
```

### Run

```bash
make run
# or
./file_manager
```

### Interactive Menu

```
================================================
     MULTITHREADED FILE MANAGEMENT SYSTEM
================================================
  1. Read File
  2. Write to File
  3. Delete File
  4. Rename File
  5. Copy File
  6. Display File Metadata
  7. Compress File (gzip)
  8. Decompress File (gzip)
  9. View Log File
 10. Exit
================================================
  Active threads: 0
  Enter choice:
```

### Clean

```bash
make clean
```

---

## Signal Handling

| Signal | Handler | Behavior |
|--------|---------|----------|
| `SIGINT` (Ctrl+C) | `sigint_handler` | Sets `g_shutdown = 1`. The main loop exits, waits for all active threads to complete, flushes logs, and releases all locks before terminating. |
| `SIGPIPE` | `SIG_IGN` | Ignored to prevent unexpected termination when writing to closed pipes. |

---

## Log File Format

All operations are logged to `logs/operations.log` with the following format:

```
[YYYY-MM-DD HH:MM:SS] [TID:thread_id] [OPERATION] filepath — STATUS — details
```

**Example log entries**:
```
[2026-04-12 17:00:00] [TID:140234] [SYSTEM] - - START - Started
[2026-04-12 17:00:05] [TID:140235] [READ] test_files/sample.txt - START -
[2026-04-12 17:00:05] [TID:140235] [READ] test_files/sample.txt - SUCCESS -
[2026-04-12 17:00:10] [TID:140236] [WRITE] output.txt - START - mode=append
[2026-04-12 17:00:10] [TID:140236] [WRITE] output.txt - SUCCESS -
[2026-04-12 17:00:15] [TID:140237] [COMPRESS] data.txt - START -
[2026-04-12 17:00:15] [TID:140237] [COMPRESS] data.txt - SUCCESS - output: data.txt.gz
[2026-04-12 17:00:20] [TID:140234] [SYSTEM] - - STOP - Stopped
```

---

## Testing & Verification

### 1. Build Test
```bash
make clean && make
```
Verify: No compiler warnings or errors.

### 2. Concurrent Read Test
From the menu, select option **1** (Read File) multiple times quickly with the same file path. All threads should complete without corruption — the file contents should be identical across all outputs.

### 3. Write Exclusion Test
Select option **2** (Write to File) multiple times quickly. Check the output file — content should be serialized (non-interleaved), proving that the write lock enforces mutual exclusion.

### 4. Deadlock-Free Rename Test
Simultaneously rename files in opposite directions (e.g., `A→B` and `B→A`). Both operations should complete without hanging, demonstrating that alphabetical lock ordering prevents deadlock.

### 5. Compress/Decompress Roundtrip Test
```bash
# 1. Compress a file (menu option 7)
#    Input: test_files/sample.txt
#    Output: test_files/sample.txt.gz

# 2. Decompress the result (menu option 8)
#    Input: test_files/sample.txt.gz
#    Output: test_files/sample.txt

# 3. Verify integrity
diff test_files/sample.txt test_files/sample_original.txt
```

### 6. Log Integrity
Select option **9** (View Logs) to inspect the log file. Verify that every operation has a timestamped entry with a thread ID.

### 7. Graceful Shutdown
While threads are running, press `Ctrl+C`. The program should print "Shutting down gracefully...", wait for all active threads to finish, and then exit cleanly.

---

## Error Handling Matrix

| Error Condition | Handling |
|----------------|----------|
| `malloc()` fails | Print `perror()`, return immediately |
| `fopen()` fails | Log with `strerror(errno)`, release lock, free memory |
| `fread()` / `fwrite()` partial | Log write error, close files, release lock |
| `stat()` fails | Log with `strerror(errno)`, release lock |
| `remove()` fails | Log with `strerror(errno)`, release lock |
| `rename()` fails | Log with `strerror(errno)`, release both locks |
| `gzopen()` fails | Log failure, close source, release lock |
| Lock acquisition fails | Log failure, free memory, return `NULL` |
| Lock table full | Print error to stderr, return `-1` |
| `pthread_create()` fails | Print error, decrement counter, free wrapper |
