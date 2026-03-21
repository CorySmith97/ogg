#include "base.h"
#include <stdarg.h>

//log_handler *logger;

void log_handler(Log_Level level, const char *msg, va_list args)
{
    switch(level) {
        case LOG_INFO: {
            printf("[info]\t");
        } break;
        case LOG_ERROR: {
            printf("[ERROR]\t");
        } break;
        case LOG_WARN: {
            printf("[WARN]\t");
        } break;
        case LOG_DEBUG: {
            printf("[DEBUG]\t");
        } break;
    }
    vprintf(msg, args); 
    printf("\n");
    return;
}

void logger(Log_Level level, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    log_handler(level, msg, args);
    va_end(args);
}
