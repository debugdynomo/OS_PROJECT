/*
 * compression.h — File Compression & Decompression interface
 * Owner:     Member 1 (interface)  →  Member 6 implements compression.c
 *
 * Uses zlib for gzip compression/decompression.
 * -------------------------------------------------------------------
 * DO NOT MODIFY this file without coordinating with Members 1 & 6.
 */

#ifndef COMPRESSION_H
#define COMPRESSION_H

/*
 * Compress a file using gzip.
 * arg: (file_arg_t *) — contains filepath.
 * Output is written to filepath.gz
 * Lock: file_read_lock(filepath)
 */
void *thread_compress_file(void *arg);

/*
 * Decompress a .gz file.
 * arg: (file_arg_t *) — contains filepath (must end in .gz).
 * Output is written to filepath with .gz stripped.
 * Lock: file_read_lock(filepath)
 */
void *thread_decompress_file(void *arg);

#endif /* COMPRESSION_H */
