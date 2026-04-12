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


String8 str8_fmt_alloc(const char *fmt, ...)
{
    String8 result = {0};
    if (!fmt) return result;

    va_list args;
    va_start(args, fmt);
    va_list args2;
    va_copy(args2, args);

    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (n < 0) {
        va_end(args2);
        return result;
    }

    char *buf = malloc((size_t)n + 1);
    if (!buf) {
        va_end(args2);
        return result;
    }

    vsnprintf(buf, (size_t)n + 1, fmt, args2);
    va_end(args2);

    result.data = (u8 *)buf;
    result.len  = (u32)n;
    return result;
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

static bool write_bytes(FILE *f, const void *data, size_t size) {
    return fwrite(data, 1, size, f) == size;
}

static bool read_bytes(FILE *f, void *data, size_t size) {
    return fread(data, 1, size, f) == size;
}

// Writes a null-terminated string as [u32 length | chars (no null)]
static bool write_string(FILE *f, const char *str) {
    uint32_t len = str ? (uint32_t)strlen(str) : 0;
    if (!write_bytes(f, &len, sizeof(len))) return false;
    if (len > 0 && !write_bytes(f, str, len))  return false;
    return true;
}

// Reads back a string into a caller-supplied buffer.
// Returns false on error or overflow.
static bool read_string(FILE *f, char *buf, size_t buf_size) {
    uint32_t len;
    if (!read_bytes(f, &len, sizeof(len))) return false;
    if (len >= buf_size)                   return false; // won't fit
    if (len > 0 && !read_bytes(f, buf, len)) return false;
    buf[len] = '\0';
    return true;
}
