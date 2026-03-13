#include "platform_sdl.h"


void 
platform_handle_events(bool *quit)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                               *quit = true;
                           } break;
            case SDL_KEYDOWN: {
                                  //keydown[event.key.keysym.scancode] = 1;
                              } break;
            case SDL_KEYUP: {
                                //keydown[event.key.keysym.scancode] = 0;
                            } break;
            default:
        }
    }
}
