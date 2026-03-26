#include "platform_sdl.h"

bool is_key_down(int key);
bool is_key_released(int key);
void on_key_down(int key);
void on_key_up(int key);
void on_mouse_moved(float x, float y, float dx, float dy);

#define MAX_KEYS 512

typedef struct KeyboardState {
    bool key_curr_state[MAX_KEYS];
    bool key_previous_state[MAX_KEYS];
    bool key_pressed[MAX_KEYS];
} KeyboardState;

static KeyboardState keyboard_state;

void platform_init(const char *name, uint32_t width, uint32_t height)
{
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) != SDL_FALSE)
    {
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

void platform_handle_events(bool *quit)
{
    if (platform_ctx.mouse_enabled) {
        SDL_ShowCursor(SDL_ENABLE);
    } else {
        // TODO add a way to lock mouse to screen.
        SDL_ShowCursor(SDL_DISABLE);
    }
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
                *quit = true;
                break;
            case SDL_KEYDOWN:
                //on_key_down(event.key.keysym.scancode);
                break;
            case SDL_KEYUP:
                //on_key_up(event.key.keysym.scancode);
                break;
            case SDL_MOUSEMOTION:
                on_mouse_moved(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
                break;
        default:
        }
    }
    platform_ctx.keystate = SDL_GetKeyboardState(NULL);
}

void platform_deinit(void)
{
    SDL_DestroyWindow(platform_ctx.window);
    SDL_Quit();
}

void platform_present()
{
    void *texpixels;
    int pitch;
    SDL_LockTexture(platform_ctx.texture, NULL, &texpixels, &pitch);
    memcpy(texpixels, renderer.pixels, renderer.width * renderer.height * sizeof(uint32_t));
    SDL_UnlockTexture(platform_ctx.texture);

    SDL_RenderClear(platform_ctx.renderer);
    SDL_RenderCopy(platform_ctx.renderer, platform_ctx.texture, NULL, NULL);
    SDL_RenderPresent(platform_ctx.renderer);
}

typedef enum {
    KEY_A = SDL_SCANCODE_A,
    KEY_B = SDL_SCANCODE_B,
    KEY_C = SDL_SCANCODE_C,
    KEY_D = SDL_SCANCODE_D,
    KEY_E = SDL_SCANCODE_E,
    KEY_F = SDL_SCANCODE_F,
    KEY_G = SDL_SCANCODE_G,
    KEY_H = SDL_SCANCODE_H,
    KEY_I = SDL_SCANCODE_I,
    KEY_J = SDL_SCANCODE_J,
    KEY_K = SDL_SCANCODE_K,
    KEY_L = SDL_SCANCODE_L,
    KEY_M = SDL_SCANCODE_M,
    KEY_N = SDL_SCANCODE_N,
    KEY_O = SDL_SCANCODE_O,
    KEY_P = SDL_SCANCODE_P,
    KEY_Q = SDL_SCANCODE_Q,
    KEY_R = SDL_SCANCODE_R,
    KEY_S = SDL_SCANCODE_S,
    KEY_T = SDL_SCANCODE_T,
    KEY_U = SDL_SCANCODE_U,
    KEY_V = SDL_SCANCODE_V,
    KEY_W = SDL_SCANCODE_W,
    KEY_X = SDL_SCANCODE_X,
    KEY_Y = SDL_SCANCODE_Y,
    KEY_Z = SDL_SCANCODE_Z,

    KEY_ENTER = SDLK_RETURN,
    KEY_ESCAPE = SDL_SCANCODE_ESCAPE,
    KEY_SPACE = SDLK_SPACE
} Keys;

bool is_key_down(int key) {
    bool pressed = false;
    if ((key > 0) && (key < MAX_KEYS)) {
        if (platform_ctx.keystate[key])
        pressed = true;
    }
    return pressed;
}

bool is_key_released(int key) {
    bool pressed = false;
    if ((key > 0) && (key < MAX_KEYS)) {
        if ((keyboard_state.key_previous_state[key] == true) && (keyboard_state.key_curr_state[key] == false))
        pressed = true;
    }
    return pressed;
}

void on_key_down(int key) 
{
    log_info("key %d", key);
    if (keyboard_state.key_curr_state[key] == true) {
        keyboard_state.key_previous_state[key] = true;
    }
    keyboard_state.key_curr_state[key] = true;
}
void on_key_up(int key) 
{ 
    keyboard_state.key_curr_state[key] = false;
}

void on_mouse_down(int button) 
{

}

void onMouseUp(int button) 
{ 
}

V2f get_mouse_pos()
{
    return v2f(
            platform_ctx.mouse_state.mouse_pos_x,
            platform_ctx.mouse_state.mouse_pos_y);
}

V2f get_mouse_delta()
{
    return v2f(
            platform_ctx.mouse_state.mouse_pos_dx,
            platform_ctx.mouse_state.mouse_pos_dy);
}

void on_mouse_moved(float x, float y, float dx, float dy) 
{ 
    platform_ctx.mouse_state.mouse_pos_x = x;
    platform_ctx.mouse_state.mouse_pos_y = y;
    platform_ctx.mouse_state.mouse_pos_dx = dx;
    platform_ctx.mouse_state.mouse_pos_dy = dy;
}

void set_escape_quit(bool *quit)
{
    if (is_key_down(KEY_ESCAPE)) {
        log_info("Hello");
        *quit = true;
    }
}
