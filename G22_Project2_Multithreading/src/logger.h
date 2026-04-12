#ifndef LOGGER_H
#define LOGGER_H

void logger_init(const char *log_file);

void log_operation(const char *operation,
                   const char *filename,
                   const char *status,
                   const char *details);

void logger_close(void);

#endif 