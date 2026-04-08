/*
 * file_ops.h — File operation thread functions
 * Owner:     Member 1 (interface)
 * Implementers:
 *   - Member 2 → read_write.c     (thread_read_file, thread_write_file)
 *   - Member 3 → delete_rename.c  (thread_delete_file, thread_rename_file)
 *   - Member 4 → copy_metadata.c  (thread_copy_file, thread_display_metadata)
 *
 * Each function is a pthread start routine:  void* func(void* arg)
 * The arg types are defined in common.h.
 * -------------------------------------------------------------------
 * DO NOT MODIFY this file without coordinating with Member 1.
 */

#ifndef FILE_OPS_H
#define FILE_OPS_H

/* ── Member 2: read_write.c ───────────────────────────────────── */

/*
 * Read a file's contents and print to stdout.
 * arg: (file_arg_t *)  — contains filepath.
 * Lock: file_read_lock()
 */
void *thread_read_file(void *arg);

/*
 * Write/append content to a file.
 * arg: (write_arg_t *) — contains filepath, content, append flag.
 * Lock: file_write_lock()
 */
void *thread_write_file(void *arg);

/* ── Member 3: delete_rename.c ────────────────────────────────── */

/*
 * Delete a file from the filesystem.
 * arg: (file_arg_t *)  — contains filepath.
 * Lock: file_write_lock()
 */
void *thread_delete_file(void *arg);

/*
 * Rename a file.
 * arg: (rename_arg_t *) — contains old_path, new_path.
 * Lock: file_write_lock() on BOTH paths (alphabetical order to avoid deadlock).
 */
void *thread_rename_file(void *arg);

/* ── Member 4: copy_metadata.c ────────────────────────────────── */

/*
 * Copy a file from src to dst.
 * arg: (copy_arg_t *) — contains src_path, dst_path.
 * Lock: file_read_lock(src), file_write_lock(dst).
 */
void *thread_copy_file(void *arg);

/*
 * Display file metadata (size, timestamps, permissions).
 * arg: (file_arg_t *)  — contains filepath.
 * Lock: file_read_lock()
 */
void *thread_display_metadata(void *arg);

#endif /* FILE_OPS_H */
