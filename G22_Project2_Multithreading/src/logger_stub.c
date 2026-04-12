#include "logger.h"
#include <stdio.h>

void logger_init(const char *log_file) {
    printf("[STUB] logger_init called with %s\n", log_file);
}

void log_operation(const char *operation, const char *filename,
                   const char *status, const char *details) {
    printf("[LOG] %s on %s - %s: %s\n",
           operation, filename, status, details);
}

void logger_close(void) {
    printf("[STUB] logger_close called\n");
}