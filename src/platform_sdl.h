#ifndef PLATFORM_SDL_H
#define PLATFORM_SDL_H

#define MAX_KEYBOARD 512

typedef struct MouseState {
    float mouse_pos_x;
    float mouse_pos_y;
    float mouse_pos_dx;
    float mouse_pos_dy;
    bool mouse_button_state;
} MouseState;

static struct {
    SDL_Window      *window;
    SDL_Renderer    *renderer;
    SDL_Texture     *texture;
    uint32_t        width;
    uint32_t        height;
    const uint8_t   *keystate;
    MouseState      mouse_state;
    bool            mouse_enabled;
} platform_ctx = {
    .mouse_enabled = true,
};


typedef enum {
    MOUSEBUTTON_LEFT = 0x0,
    MOUSEBUTTON_RIGHT = 0x1,
    MOUSEBUTTON_MIDDLE = 0x2,
    MOUSEBUTTON_INVALID = 0x100,
} MouseCode;

void present(void);
void platform_init(const char *name, uint32_t width, uint32_t height);
void platform_handle_events(bool *quit);
void platform_deinit(void);
void platform_present(void);
bool is_key_down(int key);
bool is_key_released(int key);
void set_escape_quit(bool *quit);

#endif // PLATFORM_SDL_H
