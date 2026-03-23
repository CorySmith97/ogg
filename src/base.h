#ifndef BASE_H
#define BASE_H

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include <string.h>
#include <sys/types.h>

#define UNUSED(VAL) (void)(VAL)

#define min(val1, val2) ((val1 < val2) ? val1 : val2)
#define max(val1, val2) ((val1 > val2) ? val1 : val2)

typedef uint32_t Log_Level;
enum
{
    LOG_INFO,
    LOG_ERROR,
    LOG_WARN,
    LOG_DEBUG,
};

// @todo:cs add a log handler for logging.

//typedef (*log_handler)(Log_Level level, const char *msg, ...) log_handler;

void log_handler(Log_Level level, const char *msg, va_list args);
void logger(Log_Level level, const char *msg, ...);

#define log_info(msg, ...) logger(LOG_INFO, (msg), ##__VA_ARGS__)
#define log_error(msg, ...) logger(LOG_ERROR, (msg), ##__VA_ARGS__)
#define log_debug(msg, ...) logger(LOG_DEBUG, (msg), ##__VA_ARGS__)
#define log_warn(msg, ...) logger(LOG_WARN, (msg), ##__VA_ARGS__)

#endif // BASE_H
