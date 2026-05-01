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
#include <assert.h>

#include <time.h>

#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>

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

#define Defer(n, f) for ((n);;(f))

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

#define clampf(v, mn, mx) ((v) < (mn) ? (mn) : (v) > (mx) ? (mx) : (v))

#define M_TAU 2*M_PI

#define array_push(a, v)                                        \
    do {                                                        \
        if (((a)->len + 1 < (a)->capacity)) {               \
            (a)->data[(a)->len] = v;                            \
            (a)->len += 1;                                      \
        } else {                                                \
            u32 new_cap = (a)->capacity ? ((a)->capacity * 2) : 2;\
            (a)->data = realloc((a)->data, new_cap * sizeof(v));\
            (a)->capacity = new_cap;                            \
            (a)->data[(a)->len] = v;                            \
            (a)->len++;                                         \
        }                                                       \
    } while (0);

#define internal static

#define AlignPow2(x,b)     (((x) + (b) - 1)&(~((b) - 1)))

// ARENAS AND THESE DS MACROS ARE TAKEN FROM RADDEBUGGER

//- rjf: doubly-linked-lists
#define DLLInsert_NPZ(nil,f,l,p,n,next,prev) (CheckNil(nil,f) ? \
((f) = (l) = (n), SetNil(nil,(n)->next), SetNil(nil,(n)->prev)) :\
CheckNil(nil,p) ? \
((n)->next = (f), (f)->prev = (n), (f) = (n), SetNil(nil,(n)->prev)) :\
((p)==(l)) ? \
((l)->next = (n), (n)->prev = (l), (l) = (n), SetNil(nil, (n)->next)) :\
(((!CheckNil(nil,p) && CheckNil(nil,(p)->next)) ? (0) : ((p)->next->prev = (n))), ((n)->next = (p)->next), ((p)->next = (n)), ((n)->prev = (p))))
#define DLLPushBack_NPZ(nil,f,l,n,next,prev) DLLInsert_NPZ(nil,f,l,l,n,next,prev)
#define DLLPushFront_NPZ(nil,f,l,n,next,prev) DLLInsert_NPZ(nil,l,f,f,n,prev,next)
#define DLLRemove_NPZ(nil,f,l,n,next,prev) (((n) == (f) ? (f) = (n)->next : (0)),\
((n) == (l) ? (l) = (l)->prev : (0)),\
(CheckNil(nil,(n)->prev) ? (0) :\
((n)->prev->next = (n)->next)),\
(CheckNil(nil,(n)->next) ? (0) :\
((n)->next->prev = (n)->prev)))

//- rjf: singly-linked, doubly-headed lists (queues)
#define SLLQueuePush_NZ(nil,f,l,n,next) (CheckNil(nil,f)?\
((f)=(l)=(n),SetNil(nil,(n)->next)):\
((l)->next=(n),(l)=(n),SetNil(nil,(n)->next)))
#define SLLQueuePushFront_NZ(nil,f,l,n,next) (CheckNil(nil,f)?\
((f)=(l)=(n),SetNil(nil,(n)->next)):\
((n)->next=(f),(f)=(n)))
#define SLLQueuePop_NZ(nil,f,l,next) ((f)==(l)?\
(SetNil(nil,f),SetNil(nil,l)):\
((f)=(f)->next))

//- rjf: singly-linked, singly-headed lists (stacks)
#define SLLStackPush_N(f,n,next) ((n)->next=(f), (f)=(n))
#define SLLStackPop_N(f,next) ((f)=(f)->next)

//- rjf: doubly-linked-list helpers
#define DLLInsert_NP(f,l,p,n,next,prev) DLLInsert_NPZ(0,f,l,p,n,next,prev)
#define DLLPushBack_NP(f,l,n,next,prev) DLLPushBack_NPZ(0,f,l,n,next,prev)
#define DLLPushFront_NP(f,l,n,next,prev) DLLPushFront_NPZ(0,f,l,n,next,prev)
#define DLLRemove_NP(f,l,n,next,prev) DLLRemove_NPZ(0,f,l,n,next,prev)
#define DLLInsert(f,l,p,n) DLLInsert_NPZ(0,f,l,p,n,next,prev)
#define DLLPushBack(f,l,n) DLLPushBack_NPZ(0,f,l,n,next,prev)
#define DLLPushFront(f,l,n) DLLPushFront_NPZ(0,f,l,n,next,prev)
#define DLLRemove(f,l,n) DLLRemove_NPZ(0,f,l,n,next,prev)

//- rjf: singly-linked, doubly-headed list helpers
#define SLLQueuePush_N(f,l,n,next) SLLQueuePush_NZ(0,f,l,n,next)
#define SLLQueuePushFront_N(f,l,n,next) SLLQueuePushFront_NZ(0,f,l,n,next)
#define SLLQueuePop_N(f,l,next) SLLQueuePop_NZ(0,f,l,next)
#define SLLQueuePush(f,l,n) SLLQueuePush_NZ(0,f,l,n,next)
#define SLLQueuePushFront(f,l,n) SLLQueuePushFront_NZ(0,f,l,n,next)
#define SLLQueuePop(f,l) SLLQueuePop_NZ(0,f,l,next)

//- rjf: singly-linked, singly-headed list helpers
#define SLLStackPush(f,n) SLLStackPush_N(f,n,next)
#define SLLStackPop(f) SLLStackPop_N(f,next)

#define MemoryZero(s,z)       memset((s),0,(z))

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

#ifdef DARWIN
#   define PAGE_SIZE 16384
#else
#   define PAGE_SIZE 4096
#endif
#define ARENA_HEADER_SIZE 128

// TAKEN FROM RADDEBUGGER
// ty Ryan

typedef enum {
  ARENAFLAG_NOCHAIN    = (1<<0),
  ARENAFLAG_LARGEPAGES = (1<<1),
} ArenaFlags;

typedef struct ArenaParams {
  ArenaFlags flags;
  u64 reserve_size;
  u64 commit_size;
  void *optional_backing_buffer;
  char *allocation_site_file;
  int allocation_site_line;
  char *name;
} ArenaParams;

typedef struct {
  struct Arena *prev;    // previous arena in chain
  struct Arena *current; // current arena in chain
  ArenaFlags flags;
  u64 cmt_size;
  u64 res_size;
  u64 base_pos;
  u64 pos;
  u64 cmt;
  u64 res;
  char *allocation_site_file;
  s32 allocation_site_line;
  char *name;
#if ARENA_FREE_LIST
  struct Arena *free_last;
#endif
} Arena;

typedef struct {
  Arena *arena;
  u64 pos;
} Temp;

u64 arena_default_reserve_size = MB(64);
u64 arena_default_commit_size  = KB(64);
ArenaFlags arena_default_flags = 0;

//- rjf: arena creation/destruction
Arena *arena_alloc_(ArenaParams *params);
#define arena_alloc(...) arena_alloc_(&(ArenaParams){.reserve_size = arena_default_reserve_size, .commit_size = arena_default_commit_size, .flags = arena_default_flags, .allocation_site_file = __FILE__, .allocation_site_line = __LINE__, __VA_ARGS__})
void arena_release(Arena *arena);

//- rjf: arena push/pop/pos core functions
void *arena_push(Arena *arena, u64 size, u64 align, b32 zero);
u64   arena_pos(Arena *arena);
void  arena_pop_to(Arena *arena, u64 pos);

//- rjf: arena push/pop helpers
void arena_clear(Arena *arena);
void arena_pop(Arena *arena, u64 amt);

//- rjf: temporary arena scopes
Temp temp_begin(Arena *arena);
static void temp_end(Temp temp);

//- rjf: push helper macros
#define push_array_no_zero_aligned(a, T, c, align) (T *)arena_push((a), sizeof(T)*(c), (align), (0))
#define push_array_aligned(a, T, c, align) (T *)arena_push((a), sizeof(T)*(c), (align), (1))
#define push_array_no_zero(a, T, c) push_array_no_zero_aligned(a, T, c, max(8, _Alignof(T)))
#define push_array(a, T, c) push_array_aligned(a, T, c, max(8, _Alignof(T)))
#define arena_push_struct(a, T) push_array_aligned(a, T, 1, max(8, _Alignof(T)))


typedef struct {
    u8 *data;
    u32 len;
} String8;

#define str8_lit(String) (String8){(u8*)(String), sizeof(String) - 1}

// @todo:cs Implement with arenas
String8 str8_fmt_alloc(const char *fmt, ...);
// @todo:cs arena. String that is split should be an allocated string8
String8 str8_split(String8 str, char delimeter);
String8 open_and_read_entire_file(Arena *arena, const char *file_name);

// @todo:cs Implement String8
s32 measure_text(const char *str);


#define Catelog(T) struct{String8 key; T* value;}


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
#define COLOR_PURPLE (Color){.r = 245, .g = 39, .b = 245, .a = 255}
#define COLOR_YELLOW (Color){.r = 0x50, .g = 0x50, .a = 255}

#endif // BASE_H
