/*
 * delete_rename.c — File Deletion & Renaming
 * Owner: Member 3
 *
 * ╔══════════════════════════════════════════════════════════════╗
 * ║  STUB — Compiles and runs, but prints "not implemented".   ║
 * ║  Member 3: Replace the function bodies with your real      ║
 * ║  implementations. Use sync.h for locking, logger.h for     ║
 * ║  logging.                                                   ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include "file_ops.h"
#include "common.h"
#include "sync.h"
#include "logger.h"

void *thread_delete_file(void *arg)
{
    file_arg_t *fa = (file_arg_t *)arg;

    log_operation("DELETE", fa->filepath, "START", NULL);

    /* Acquire exclusive lock */
    if (file_write_lock(fa->filepath) != 0) {
        log_operation("DELETE", fa->filepath, "FAILURE", "Could not acquire write lock");
        free(fa);
        return NULL;
    }

    /* ── Member 3: implement file deletion here ────────────── */
    if (remove(fa->filepath) != 0) {
        log_operation("DELETE", fa->filepath, "FAILURE", strerror(errno));
        file_unlock(fa->filepath);
        free(fa);
        return NULL;
    }
    /* ── End Member 3 section ───────────────────────────────── */

    file_unlock(fa->filepath);
    log_operation("DELETE", fa->filepath, "SUCCESS", NULL);

    free(fa);
    return NULL;
}

void *thread_rename_file(void *arg)
{
    rename_arg_t *ra = (rename_arg_t *)arg;
    char detail[MAX_PATH_LEN * 2 + 8];
    snprintf(detail, sizeof(detail), "%s -> %s", ra->old_path, ra->new_path);

    log_operation("RENAME", ra->old_path, "START", detail);

    /*
     * Lock in alphabetical order to prevent deadlock when two
     * renames cross paths, e.g. rename(A,B) and rename(B,A).
     */
    const char *first  = ra->old_path;
    const char *second = ra->new_path;
    if (strcmp(first, second) > 0) {
        first  = ra->new_path;
        second = ra->old_path;
    }

    if (file_write_lock(first) != 0) {
        log_operation("RENAME", ra->old_path, "FAILURE", "Lock failed (first)");
        free(ra);
        return NULL;
    }
    if (file_write_lock(second) != 0) {
        file_unlock(first);
        log_operation("RENAME", ra->old_path, "FAILURE", "Lock failed (second)");
        free(ra);
        return NULL;
    }

    /* ── Member 3: implement rename here ───────────────────── */
    if (rename(ra->old_path, ra->new_path) != 0) {
        log_operation("RENAME", ra->old_path, "FAILURE", strerror(errno));
        file_unlock(second);
        file_unlock(first);
        free(ra);
        return NULL;
    }
    /* ── End Member 3 section ───────────────────────────────── */

    file_unlock(second);
    file_unlock(first);
    log_operation("RENAME", ra->new_path, "SUCCESS", detail);

    free(ra);
    return NULL;
}
