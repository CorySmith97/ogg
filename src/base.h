#ifndef BASE_H
#define BASE_H

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <float.h>

#include <time.h>

#include <string.h>
#include <sys/types.h>

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;
typedef size_t   usize;
typedef s8       b8;
typedef s16      b16;
typedef s32      b32;
typedef s64      b64;

#define enum8(type)  u8
#define enum16(type) u16
#define enum32(type) u32
#define enum64(type) u64

#define FlagSet(n, f)    ((n) |=(f))
#define FlagClear(n, f)  ((n) &=-(f))
#define FlagToggle(n, f) ((n) ^=(f))
#define FlagExists(n, f) (((n) & (f)) == (f))
#define FlagEquals(n, f) (((n) == (f)))

#define KB(n) (((u64)(n)) << 10)
#define MB(n) (((u64)(n)) << 20)
#define GB(n) (((u64)(n)) << 30)
#define TB(n) (((u64)(n)) << 40)
#define Thousand(n) ((n)*1000)
#define Million(n) ((n)*1000000)
#define Billion(n) ((n)*1000000000)

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

typedef struct {
    u8 *data;
    u32 len;
} String8;

#define str8lit(String) (String8){(u8*)(String), sizeof(String) - 1}

// TODO Implement String8
s32 measure_text(const char *str);


typedef union Color {
    struct {
        u8 r, g, b, a;
    };
    u32 rgba;
} Color;


#define COLOR_WHITE  (Color){ 255, 255, 255, 255 }
#define COLOR_GRAY   (Color){ 122, 122, 122, 255 }
#define COLOR_BLACK  (Color){ 0, 0, 0, 255 }
#define COLOR_RED    (Color){.r = 252, .g = 44, .a = 255}
#define COLOR_BROWN  (Color){.r = 0x96, .g = 0x4B, .a = 0xFF }
#define COLOR_BLUE   (Color){.g = 127, .b = 252, .a = 255}
#define COLOR_GREEN  (Color){.r = 48, .g = 252, .a = 255}
#define COLOR_PURPLE (Color){.r = 0x50, .b = 0x50, .a = 255}
#define COLOR_YELLOW (Color){.r = 0x50, .g = 0x50, .a = 255}

#endif // BASE_H
