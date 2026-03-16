#ifndef PLATFORM_SDL_H
#define PLATFORM_SDL_H

static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t width;
    uint32_t height;
} platform_ctx = {
};

void present(void);
void platform_init(const char *name, uint32_t width, uint32_t height);
void platform_handle_events(bool *quit);
void platform_deinit(void);
void platform_present(void);

#endif // PLATFORM_SDL_H
