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

#define internal static
#define global   static

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

#define log_info(msg, ...)  logger(LOG_INFO,  (msg), ##__VA_ARGS__)
#define log_error(msg, ...) logger(LOG_ERROR, (msg), ##__VA_ARGS__)
#define log_debug(msg, ...) logger(LOG_DEBUG, (msg), ##__VA_ARGS__)
#define log_warn(msg, ...)  logger(LOG_WARN,  (msg), ##__VA_ARGS__)

typedef struct {
} Arena_Params;

typedef struct Arena {
    struct Arena *next;
    struct Arena *prev;
    size_t reserved;
    size_t committed;
    uintptr_t position;
    uintptr_t checkpoint;
    void  *data;
} Arena;

Arena *_arena_alloc(Arena_Params params);
void   arena_checkpoint(Arena *arena);
void   arena_reset_to_checkpoint(Arena *arena);
void   arena_reset(Arena *arena);
void  *arena_push(Arena *arena);

#define arena_alloc(...) _arena_alloc((Arena_Params){})

#endif // BASE_H
