#ifndef FILE_OPS_H
#define FILE_OPS_H

void *thread_read_file(void *arg);
void *thread_write_file(void *arg);
void *thread_delete_file(void *arg);
void *thread_rename_file(void *arg);
void *thread_copy_file(void *arg);
void *thread_display_metadata(void *arg);

#endif
