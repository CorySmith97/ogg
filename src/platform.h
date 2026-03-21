#ifndef PLATFORM_H
#define PLATFORM_H

#if PLATFORM_SDL
#include "platform_sdl.h"
#endif

#if PLATFORM_MACOS
#include "platform_macos.h"
#endif

#endif // PLATFORM_H
