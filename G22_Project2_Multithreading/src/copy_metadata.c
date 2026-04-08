/*
 * copy_metadata.c — File Copying & Metadata Display
 * Owner: Member 4
 *
 * ╔══════════════════════════════════════════════════════════════╗
 * ║  STUB — Compiles and runs, but prints "not implemented".   ║
 * ║  Member 4: Replace the function bodies with your real      ║
 * ║  implementations. Use sync.h for locking, logger.h for     ║
 * ║  logging.                                                   ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include "file_ops.h"
#include "common.h"
#include "sync.h"
#include "logger.h"

void *thread_copy_file(void *arg)
{
    copy_arg_t *ca = (copy_arg_t *)arg;
    char detail[MAX_PATH_LEN * 2 + 8];
    snprintf(detail, sizeof(detail), "%s -> %s", ca->src_path, ca->dst_path);

    log_operation("COPY", ca->src_path, "START", detail);

    /* Read-lock source, write-lock destination */
    if (file_read_lock(ca->src_path) != 0) {
        log_operation("COPY", ca->src_path, "FAILURE", "Lock failed (source)");
        free(ca);
        return NULL;
    }

    /* ── Member 4: implement file copy here ────────────────── */
    FILE *src = fopen(ca->src_path, "rb");
    if (!src) {
        log_operation("COPY", ca->src_path, "FAILURE", strerror(errno));
        file_unlock(ca->src_path);
        free(ca);
        return NULL;
    }

    FILE *dst = fopen(ca->dst_path, "wb");
    if (!dst) {
        log_operation("COPY", ca->dst_path, "FAILURE", strerror(errno));
        fclose(src);
        file_unlock(ca->src_path);
        free(ca);
        return NULL;
    }

    char buf[BUFFER_SIZE];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (fwrite(buf, 1, n, dst) != n) {
            log_operation("COPY", ca->dst_path, "FAILURE", "Write error during copy");
            fclose(src);
            fclose(dst);
            file_unlock(ca->src_path);
            free(ca);
            return NULL;
        }
    }

    fclose(src);
    fclose(dst);
    /* ── End Member 4 section ───────────────────────────────── */

    file_unlock(ca->src_path);
    log_operation("COPY", ca->dst_path, "SUCCESS", detail);

    free(ca);
    return NULL;
}

void *thread_display_metadata(void *arg)
{
    file_arg_t *fa = (file_arg_t *)arg;

    log_operation("METADATA", fa->filepath, "START", NULL);

    if (file_read_lock(fa->filepath) != 0) {
        log_operation("METADATA", fa->filepath, "FAILURE", "Could not acquire read lock");
        free(fa);
        return NULL;
    }

    /* ── Member 4: implement metadata display here ─────────── */
    struct stat st;
    if (stat(fa->filepath, &st) != 0) {
        log_operation("METADATA", fa->filepath, "FAILURE", strerror(errno));
        file_unlock(fa->filepath);
        free(fa);
        return NULL;
    }

    printf("\n--- Metadata for '%s' ---\n", fa->filepath);
    printf("  Size          : %ld bytes\n", (long)st.st_size);
    printf("  Permissions   : %o\n", st.st_mode & 0777);
    printf("  Last accessed : %s", ctime(&st.st_atime));
    printf("  Last modified : %s", ctime(&st.st_mtime));
    printf("  Last changed  : %s", ctime(&st.st_ctime));
    printf("--- End metadata ---\n\n");
    /* ── End Member 4 section ───────────────────────────────── */

    file_unlock(fa->filepath);
    log_operation("METADATA", fa->filepath, "SUCCESS", NULL);

    free(fa);
    return NULL;
}
