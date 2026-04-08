/*
 * logger.h — Thread-safe logging interface
 * Owner:     Member 1 (interface)  →  Member 5 implements logger.c
 *
 * All modules call these functions. Member 5 provides the real
 * implementation in logger.c — do NOT modify this header.
 * -------------------------------------------------------------------
 * DO NOT MODIFY this file without coordinating with Members 1 & 5.
 */

#ifndef LOGGER_H
#define LOGGER_H

/*
 * Initialize the logger.  Opens the log file and initialises
 * the internal mutex.  Call once at program start.
 * Returns 0 on success, -1 on failure.
 */
int  logger_init(const char *log_file_path);

/*
 * Write a timestamped, thread-ID-tagged log line.
 *
 *   operation  — e.g. "READ", "WRITE", "DELETE", "COMPRESS"
 *   filepath   — file being operated on
 *   status     — "SUCCESS", "FAILURE", "START", etc.
 *   detail     — optional free-form message (may be NULL)
 *
 * Format:
 *   [YYYY-MM-DD HH:MM:SS] [TID:nnn] [OPERATION] filepath — status — detail
 */
void log_operation(const char *operation,
                   const char *filepath,
                   const char *status,
                   const char *detail);

/*
 * Flush and close the log file.  Call once at program shutdown.
 */
void logger_close(void);

#endif /* LOGGER_H */
