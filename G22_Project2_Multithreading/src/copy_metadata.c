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

    if (file_read_lock(ca->src_path) != 0) {
        log_operation("COPY", ca->src_path, "FAILURE", "Lock failed");
        free(ca);
        return NULL;
    }

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
            log_operation("COPY", ca->dst_path, "FAILURE", "Write error");
            fclose(src);
            fclose(dst);
            file_unlock(ca->src_path);
            free(ca);
            return NULL;
        }
    }

    fclose(src);
    fclose(dst);
    
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

    file_unlock(fa->filepath);
    log_operation("METADATA", fa->filepath, "SUCCESS", NULL);

    free(fa);
    return NULL;
}
