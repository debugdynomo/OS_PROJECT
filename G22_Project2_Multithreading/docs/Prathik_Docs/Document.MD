🔹 Project 2: Multithreaded File Management System

Overview

This project implements a multithreaded file management system in C using POSIX threads (pthreads). The objective is to handle concurrent file operations safely using synchronization techniques.

The system ensures:

- Multiple threads can read a file simultaneously
- Only one thread can write to a file at a time
- No data corruption occurs due to race conditions

---

Features Implemented (Member 2)

Concurrent File Reading

- Multiple threads can read the same file simultaneously
- Uses shared read locks ("pthread_rwlock_rdlock")
- Ensures no writer is active during reading

---

Exclusive File Writing

- Only one thread can write to a file at a time
- Uses exclusive write locks ("pthread_rwlock_wrlock")
- Blocks readers and other writers during write

---

Design and Implementation

🔹 Read Operation

void *thread_read_file(void *arg)

- Acquires read lock using "file_read_lock()"
- Opens file in read mode
- Reads content line by line
- Releases lock after completion

---

🔹 Write Operation

void *thread_write_file(void *arg)

- Acquires write lock using "file_write_lock()"
- Opens file in append/overwrite mode
- Writes content safely
- Releases lock after completion

---

🔹 Lock Table Mechanism

typedef struct {
    char filename[MAX_FILENAME_LEN];
    pthread_rwlock_t rwlock;
    int ref_count;
    int in_use;
} FileLock;

- Maintains one lock per file
- Prevents conflicts between threads

---

Execution Flow

1. Thread requests access to a file
2. Lock is acquired (read/write)
3. File operation is performed
4. Lock is released
5. Operation is logged

---

File Structure

- "file_ops.c" — Read/Write logic
- "sync.c" — Lock management
- "logger_stub.c" — Logging
- "test_file_ops.c" — Testing

---

Testing

🔹 Test 1: Concurrent Reads

- Multiple readers access file simultaneously
- Output is interleaved

🔹 Test 2: Exclusive Writes

- Only one writer executes at a time
- Other writers wait

---

Output / Results

Execution Output

"Execution Output" (./images/output.png)

---

Explanation of Output

Concurrent Reads

- All three readers acquired the lock simultaneously
- Lines are printed in interleaved manner
- Confirms true parallel reading

Exclusive Writes

- Writer 1 acquires lock first
- Writer 2 and 3 wait
- Each writer executes one after another
- Confirms mutual exclusion

---

Results Summary

- Concurrent reads working correctly
- Exclusive writes enforced properly
- No data corruption observed
- Logging working as expected

---

Limitations

- Logging is basic (stub implementation)
- Fixed lock table size
- No advanced UI

---

Future Enhancements

- Add file delete and rename operations
- Add compression features
- Improve logging system
- Build interactive CLI

---

Concepts Learned

- Multithreading using pthreads
- Read-write locks
- Synchronization and race conditions
- Thread-safe programming

---

Conclusion

The project successfully demonstrates safe concurrent file access using read-write locks. It achieves parallelism for read operations and ensures correctness for write operations.

---

References

- POSIX Threads Documentation
- Linux Man Pages ("pthread_rwlock")
- Operating Systems Course Material

---
