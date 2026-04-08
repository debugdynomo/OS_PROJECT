/*
 * read_write.c — Concurrent Read & Exclusive Write
 * Owner: Member 2
 *
 * ╔══════════════════════════════════════════════════════════════╗
 * ║  STUB — Compiles and runs, but prints "not implemented".   ║
 * ║  Member 2: Replace the function bodies with your real      ║
 * ║  implementations. Use sync.h for locking, logger.h for     ║
 * ║  logging.                                                   ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include "file_ops.h"
#include "common.h"
#include "sync.h"
#include "logger.h"

void *thread_read_file(void *arg)
{
    file_arg_t *fa = (file_arg_t *)arg;

    log_operation("READ", fa->filepath, "START", NULL);

    /* Acquire read lock (multiple readers allowed) */
    if (file_read_lock(fa->filepath) != 0) {
        log_operation("READ", fa->filepath, "FAILURE", "Could not acquire read lock");
        free(fa);
        return NULL;
    }

    /* ── Member 2: implement file reading here ──────────────── */
    FILE *fp = fopen(fa->filepath, "r");
    if (!fp) {
        log_operation("READ", fa->filepath, "FAILURE", strerror(errno));
        file_unlock(fa->filepath);
        free(fa);
        return NULL;
    }

    char buf[BUFFER_SIZE];
    printf("\n--- Contents of '%s' ---\n", fa->filepath);
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        printf("%s", buf);
    }
    printf("--- End of '%s' ---\n\n", fa->filepath);
    fclose(fp);
    /* ── End Member 2 section ───────────────────────────────── */

    file_unlock(fa->filepath);
    log_operation("READ", fa->filepath, "SUCCESS", NULL);

    free(fa);
    return NULL;
}

void *thread_write_file(void *arg)
{
    write_arg_t *wa = (write_arg_t *)arg;

    log_operation("WRITE", wa->filepath, "START",
                  wa->append ? "mode=append" : "mode=overwrite");

    /* Acquire exclusive write lock */
    if (file_write_lock(wa->filepath) != 0) {
        log_operation("WRITE", wa->filepath, "FAILURE", "Could not acquire write lock");
        free(wa);
        return NULL;
    }

    /* ── Member 2: implement file writing here ─────────────── */
    FILE *fp = fopen(wa->filepath, wa->append ? "a" : "w");
    if (!fp) {
        log_operation("WRITE", wa->filepath, "FAILURE", strerror(errno));
        file_unlock(wa->filepath);
        free(wa);
        return NULL;
    }

    fprintf(fp, "%s", wa->content);
    fclose(fp);
    /* ── End Member 2 section ───────────────────────────────── */

    file_unlock(wa->filepath);
    log_operation("WRITE", wa->filepath, "SUCCESS", NULL);

    free(wa);
    return NULL;
}
