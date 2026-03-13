#ifndef PLATFORM_SDL_H
#define PLATFORM_SDL_H

static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} platform_ctx = {
};

void present(void);
void platform_init(const char *name, int width, int height);
void platform_handle_events(bool *quit);

#endif // PLATFORM_SDL_H
