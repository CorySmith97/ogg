#include "base.h"
#include <stdarg.h>

#define MAX_STR_LEN 255

//log_handler *logger;

void log_handler(Log_Level level, const char *msg, va_list args)
{
    switch(level) {
        case LOG_INFO: {
            printf("[INFO]\t");
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

Arena *_arena_alloc(Arena_Params params)
{
    return NULL;
}

void arena_checkpoint(Arena *arena)
{
    arena->checkpoint = arena->position;
}

void arena_reset_to_checkpoint(Arena *arena)
{
    arena->position = arena->checkpoint;
}

void arena_reset(Arena *arena)
{
    arena->position = (uintptr_t)arena->data;
}

void *arena_push(Arena *arena)
{
    return NULL;
}


s32 measure_text(const char *str)
{
    return strnlen(str, MAX_STR_LEN);
}

Color color_scale(Color c, double value)
{
    return (Color){
        c.r * value,
        c.g * value,
        c.b * value,
        c.a,
    };
}

Color color_add(Color c1, Color c2)
{
    return (Color){
        c1.r + c2.r,
        c1.g + c2.g,
        c1.b + c2.b,
        c1.a + c2.a,
    };
}

Color color_mul(Color c1, Color c2)
{
    return (Color){
        c1.r * c2.r,
        c1.g * c2.g,
        c1.b * c2.b,
        c1.a * c2.a,
    };
}

Color color_modulate(Color a, Color b)
{
    return (Color){
        (u8)(a.r * b.r / 255),
        (u8)(a.g * b.g / 255),
        (u8)(a.b * b.b / 255),
        (u8)(a.a * b.a / 255),
    };
}

Color alpha_blend(Color src, Color dst)
{
    u32 a = src.a;
    u32 ia = 255 - a;

    return (Color){
        .r = (u8)((src.r * a + dst.r * ia) / 255),
        .g = (u8)((src.g * a + dst.g * ia) / 255),
        .b = (u8)((src.b * a + dst.b * ia) / 255),
        .a = 255,
    };
}
