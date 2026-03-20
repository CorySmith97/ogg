#include "platform_sdl.h"

void
platform_init(const char *name, uint32_t width, uint32_t height)
{
    if (SDL_Init(SDL_INIT_VIDEO) != SDL_FALSE) {
        printf("Failed to initial Video\n");
        exit(EXIT_FAILURE);
    }

    platform_ctx.window = SDL_CreateWindow(name, 
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
            width, height, 
            SDL_WINDOW_SHOWN);


    platform_ctx.renderer = SDL_CreateRenderer(platform_ctx.window, 0, SDL_RENDERER_SOFTWARE);
    platform_ctx.texture = SDL_CreateTexture(platform_ctx.renderer, 
            SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 
            GAME_WIDTH, GAME_HEIGHT);
}

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

void platform_deinit(void)
{
    SDL_DestroyWindow(platform_ctx.window);
    SDL_Quit();
}

void platform_present()
{
    SDL_Rect dst;
    dst.x = 0;
    dst.y = 0;
    dst.w = platform_ctx.width;
    dst.h = platform_ctx.height;
    void *texpixels;
    int pitch;
    SDL_LockTexture(platform_ctx.texture, NULL, &texpixels, &pitch);
    memcpy(texpixels, renderer.pixels, renderer.width * renderer.height * sizeof(uint32_t));
    //printf("pixel 0: %d Pixel 100: %d\n", s->pixels[0], s->pixels[100]);
    SDL_UnlockTexture(platform_ctx.texture);

    SDL_RenderClear(platform_ctx.renderer);
    SDL_RenderCopy(platform_ctx.renderer, platform_ctx.texture, NULL, NULL);
    SDL_RenderPresent(platform_ctx.renderer);
}

bool is_key_down(int key)
{
    UNUSED(key);
    return false;
}

bool is_key_released(int key)
{
    UNUSED(key);
    return false;
}
