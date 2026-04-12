#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "file_ops.h"
#include "sync.h"
#include "logger.h"

void* thread_delete_file(void* arg) {
    file_args* args = (file_args*)arg;
    char* filename = args->filename;

    pthread_rwlock_t* lock = get_file_lock(filename);

    // Take write lock (exclusive)
    pthread_rwlock_wrlock(lock);

    int res = remove(filename);

    if (res == 0) {
        log_operation("DELETE", filename, "SUCCESS", "File deleted");
        printf("Deleted: %s\n", filename);
    } else {
        log_operation("DELETE", filename, "FAIL", strerror(errno));
        printf("Error deleting %s: %s\n", filename, strerror(errno));
    }

    pthread_rwlock_unlock(lock);
    release_file_lock(filename);

    return NULL;
}


void* thread_rename_file(void* arg) {
    rename_args* args = (rename_args*)arg;

    char* oldname = args->oldname;
    char* newname = args->newname;

    pthread_rwlock_t *lock1, *lock2;

    // Deadlock prevention: alphabetical order
    if (strcmp(oldname, newname) < 0) {
        lock1 = get_file_lock(oldname);
        lock2 = get_file_lock(newname);
    } else {
        lock1 = get_file_lock(newname);
        lock2 = get_file_lock(oldname);
    }

    pthread_rwlock_wrlock(lock1);
    pthread_rwlock_wrlock(lock2);

    int res = rename(oldname, newname);

    if (res == 0) {
        log_operation("RENAME", oldname, "SUCCESS", newname);
        printf("Renamed: %s → %s\n", oldname, newname);
    } else {
        log_operation("RENAME", oldname, "FAIL", strerror(errno));
        printf("Error renaming %s: %s\n", oldname, strerror(errno));
    }

    pthread_rwlock_unlock(lock2);
    pthread_rwlock_unlock(lock1);

    release_file_lock(oldname);
    release_file_lock(newname);

    return NULL;
}