#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <pthread.h>

void logger_init(const char *filename);
void log_operation(const char *operation, const char *filename, const char *status, const char *details);
void logger_close();

#endif
