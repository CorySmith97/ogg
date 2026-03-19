#ifndef PROF_H
#define PROF_H

#include <time.h>

typedef struct {
    const char *name;
    uint64_t start_ns;
    uint64_t end_ns;
    uint64_t delta_ns;
} Section;

typedef struct Profiler {
    // @todo:cs change to be a hashmap
    Section *sections;
} Profiler;

static Profiler profiler;

#ifdef DEBUG
#   define SectionStart(_S) _internal_section_start((_S))
#   define SectionEnd(_S) _internal_section_end((_S))
#else
#   define SectionStart(_S)
#   define SectionEnd(_S)
#endif

static uint64_t timespec_to_ns(struct timespec ts) {
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static void _internal_section_start(const char *str) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    Section d = {
        .name     = str,
        .start_ns = timespec_to_ns(ts),
        .end_ns   = 0,
        .delta_ns = 0,
    };
    arrput(profiler.sections, d);
}

static void _internal_section_end(const char *str) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t now_ns = timespec_to_ns(ts);
    
    for (int i = arrlen(profiler.sections) - 1; i >= 0; i--) {
        Section *sec = &profiler.sections[i];
        if (strcmp(str, sec->name) == 0 && sec->end_ns == 0) { 
            sec->end_ns   = now_ns;
            sec->delta_ns = now_ns - sec->start_ns;
            break;
        }
    }
}

void profiler_reset(void) 
{
    arrfree(profiler.sections);
}

void profiler_report(void)
{

    for (int i = 0; i < arrlen(profiler.sections); i++)
    {
        Section s = profiler.sections[i];
        logger(LOG_INFO, "%s: (%.3fms)", s.name, s.delta_ns / 1000000.0);
    }
}


#endif // PROF_H
