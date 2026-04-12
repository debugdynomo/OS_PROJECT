#include "compression.h"
#include "common.h"
#include "sync.h"
#include "logger.h"
#include <zlib.h>

void *thread_compress_file(void *arg)
{
    file_arg_t *fa = (file_arg_t *)arg;

    log_operation("COMPRESS", fa->filepath, "START", NULL);

    if (file_read_lock(fa->filepath) != 0) {
        log_operation("COMPRESS", fa->filepath, "FAILURE", "Could not acquire lock");
        free(fa);
        return NULL;
    }

    char out_path[MAX_PATH_LEN];
    snprintf(out_path, sizeof(out_path), "%s.gz", fa->filepath);

    FILE *src = fopen(fa->filepath, "rb");
    if (!src) {
        log_operation("COMPRESS", fa->filepath, "FAILURE", strerror(errno));
        file_unlock(fa->filepath);
        free(fa);
        return NULL;
    }

    gzFile gz = gzopen(out_path, "wb");
    if (!gz) {
        log_operation("COMPRESS", out_path, "FAILURE", "gzopen failed");
        fclose(src);
        file_unlock(fa->filepath);
        free(fa);
        return NULL;
    }

    char buf[BUFFER_SIZE];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (gzwrite(gz, buf, (unsigned)n) == 0) {
            log_operation("COMPRESS", out_path, "FAILURE", "gzwrite failed");
            gzclose(gz);
            fclose(src);
            file_unlock(fa->filepath);
            free(fa);
            return NULL;
        }
    }

    fclose(src);
    gzclose(gz);

    file_unlock(fa->filepath);

    char detail[MAX_PATH_LEN + 32];
    snprintf(detail, sizeof(detail), "output: %s", out_path);
    log_operation("COMPRESS", fa->filepath, "SUCCESS", detail);

    free(fa);
    return NULL;
}

void *thread_decompress_file(void *arg)
{
    file_arg_t *fa = (file_arg_t *)arg;

    log_operation("DECOMPRESS", fa->filepath, "START", NULL);

    if (file_read_lock(fa->filepath) != 0) {
        log_operation("DECOMPRESS", fa->filepath, "FAILURE", "Could not acquire lock");
        free(fa);
        return NULL;
    }

    char out_path[MAX_PATH_LEN];
    strncpy(out_path, fa->filepath, MAX_PATH_LEN - 1);
    out_path[MAX_PATH_LEN - 1] = '\0';

    size_t len = strlen(out_path);
    if (len > 3 && strcmp(out_path + len - 3, ".gz") == 0) {
        out_path[len - 3] = '\0';
    } else {
        snprintf(out_path, sizeof(out_path), "%s.out", fa->filepath);
    }

    gzFile gz = gzopen(fa->filepath, "rb");
    if (!gz) {
        log_operation("DECOMPRESS", fa->filepath, "FAILURE", "gzopen failed");
        file_unlock(fa->filepath);
        free(fa);
        return NULL;
    }

    FILE *dst = fopen(out_path, "wb");
    if (!dst) {
        log_operation("DECOMPRESS", out_path, "FAILURE", strerror(errno));
        gzclose(gz);
        file_unlock(fa->filepath);
        free(fa);
        return NULL;
    }

    char buf[BUFFER_SIZE];
    int bytes;
    while ((bytes = gzread(gz, buf, sizeof(buf))) > 0) {
        if (fwrite(buf, 1, (size_t)bytes, dst) != (size_t)bytes) {
            log_operation("DECOMPRESS", out_path, "FAILURE", "Write error");
            fclose(dst);
            gzclose(gz);
            file_unlock(fa->filepath);
            free(fa);
            return NULL;
        }
    }

    fclose(dst);
    gzclose(gz);

    file_unlock(fa->filepath);

    char detail[MAX_PATH_LEN + 32];
    snprintf(detail, sizeof(detail), "output: %s", out_path);
    log_operation("DECOMPRESS", fa->filepath, "SUCCESS", detail);

    free(fa);
    return NULL;
}

