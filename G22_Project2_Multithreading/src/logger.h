#ifndef LOGGER_H
#define LOGGER_H

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
