# File Deletion and Renaming

## Overview

This module implements thread-safe file deletion and renaming operations in a multithreaded file management system using POSIX threads.

---

## File Deletion

* Implemented using `thread_delete_file`
* Uses `pthread_rwlock_wrlock` for exclusive access
* Ensures no other thread accesses the file during deletion
* Deletes file using `remove()`
* Logs success or failure

---

## File Renaming

* Implemented using `thread_rename_file`
* Uses write locks on both source and destination files
* Prevents deadlocks using alphabetical lock ordering
* Renames file using `rename()`
* Logs operation result

---

## Synchronization

* Used **write locks (`wrlock`)** for both operations
* Ensures thread-safe execution
* Prevents concurrent modification issues

---

## Deadlock Prevention

* Locks are acquired in alphabetical order of filenames
* Avoids circular waiting conditions

---

## Dependencies

* `sync.h` for file locking
* `logger.h` for logging operations

---

## Conclusion

Successfully implemented safe and efficient deletion and renaming operations in a concurrent environment.
