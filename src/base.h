#ifndef BASE_H
#define BASE_H

#include <stdint.h>

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

#endif // BASE_H
