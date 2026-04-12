#ifndef LOGGER_H
#define LOGGER_H

int logger_init(const char *log_file_path);
void log_operation(const char *operation, const char *filepath, const char *status, const char *detail);
void logger_close(void);

#endif
void logger_init(const char *log_file);

void log_operation(const char *operation,
                   const char *filename,
                   const char *status,
                   const char *details);

void logger_close(void);

#endif 
/**
 * @brief Thread-safe logging function.
 * 
 * Logs operations safely with a timestamp and thread ID.
 * 
 * @param operation Name of the operation (e.g., "COMPRESS_START")
 * @param filename File being operated upon
 * @param status Status of operation ("INFO", "SUCCESS", "ERROR")
 * @param details Additional details
 */
void log_operation(const char *operation, const char *filename, const char *status, const char *details);

#endif // LOGGER_H
#include <stdio.h>
#include <pthread.h>

void logger_init(const char *filename);
void log_operation(const char *operation, const char *filename, const char *status, const char *details);
void logger_close();

#endif
