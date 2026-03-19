#ifndef PLATFORM_SDL_H
#define PLATFORM_SDL_H

#define MAX_KEYBOARD 512

static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t width;
    uint32_t height;
    uint8_t keystate[MAX_KEYBOARD];
    uint8_t prev_keystate[MAX_KEYBOARD];
} platform_ctx = {
};

void present(void);
void platform_init(const char *name, uint32_t width, uint32_t height);
void platform_handle_events(bool *quit);
void platform_deinit(void);
void platform_present(void);
bool is_key_down(int key);
bool is_key_released(int key);

#endif // PLATFORM_SDL_H
