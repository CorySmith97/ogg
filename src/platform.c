#include "platform.h"

#if PLATFORM_SDL
#include "platform_sdl.c"
#endif

#if PLATFORM_MACOS
#include "platform_macos.c"
#endif
