#ifndef PLATFORM_H
#define PLATFORM_H

#define FRAME_TIMES_NUMBER 60

typedef struct{
  f64 frame_target_time;
  f64 sleep_window;
  u64 previous_time;
  f64 delta_time;

  f64 frame_times[FRAME_TIMES_NUMBER];
  u32 frame_index;
  u32 frame_count;
  f64 frame_time_accum;

  f64 update_interval;
  f64 update_timer;
  f64 last_fps;
  b32 fps_updated;
} Timer;

void timer_init_(Timer *t, u32 refresh_rate, f64 update_interval);
#define timer_init(T, RR) timer_init_(T, RR, 1.0)
void timer_tick(Timer *t);

#if PLATFORM_SDL
#include "p_sdl.h"
#endif

#endif // PLATFORM_H
