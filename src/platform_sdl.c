#include "platform_sdl.h"

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


#define MAX_KEYBOARD_KEYS 512

typedef struct {
    bool keys_pressed[MAX_KEYBOARD_KEYS];
    bool keys_released[MAX_KEYBOARD_KEYS];
    bool prev_keys_pressed[MAX_KEYBOARD_KEYS];
    bool prev_keys_released[MAX_KEYBOARD_KEYS];
} KeyboardState;

static KeyboardState keyboard_state = {false};

void platform_handle_events(bool *quit)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        {
            *quit = true;
        }
        break;
        case SDL_KEYDOWN:
        {
                if (keyboard_state.keys_pressed[event.key.keysym.scancode]) {
                    break; // Key is already pressed, skip updating state
                } else {
                    keyboard_state.prev_keys_released[event.key.keysym.scancode] = true;
                    keyboard_state.prev_keys_pressed[event.key.keysym.scancode] = false;
                }
                keyboard_state.keys_pressed[event.key.keysym.scancode] = true;
                logger(LOG_DEBUG, "trying to log key down");
                break;
        }
        break;
        case SDL_KEYUP:
        {
                if (keyboard_state.keys_released[event.key.keysym.scancode]) {
                    break; // Key is already released, skip updating state
                } else {
                    keyboard_state.prev_keys_released[event.key.keysym.scancode] = false;
                    keyboard_state.prev_keys_pressed[event.key.keysym.scancode] = true;
                }
                keyboard_state.keys_released[event.key.keysym.scancode] = true;
                break;
        }
        break;
        default:
        }

        // Update key states
        for (int i = 0; i < MAX_KEYBOARD_KEYS; i++) {
            if (keyboard_state.prev_keys_pressed[i] != keyboard_state.keys_pressed[i]) {
                // Key was pressed, update prev keys and set released to false
                keyboard_state.prev_keys_pressed[i] = true;
                keyboard_state.prev_keys_released[i] = false;
            } else if (keyboard_state.prev_keys_released[i] != keyboard_state.keys_released[i]) {
                // Key was released, update prev keys and set pressed to false
                keyboard_state.prev_keys_released[i] = true;
                keyboard_state.prev_keys_pressed[i] = false;
            }
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
    // printf("pixel 0: %d Pixel 100: %d\n", s->pixels[0], s->pixels[100]);
    SDL_UnlockTexture(platform_ctx.texture);

    SDL_RenderClear(platform_ctx.renderer);
    SDL_RenderCopy(platform_ctx.renderer, platform_ctx.texture, NULL, NULL);
    SDL_RenderPresent(platform_ctx.renderer);
}

typedef enum {
    KEY_A = SDLK_a,
    KEY_B = SDLK_b,
    KEY_C = SDLK_c,
    KEY_D = SDLK_d,
    KEY_E = SDLK_e,
    KEY_F = SDLK_f,
    KEY_G = SDLK_g,
    KEY_H = SDLK_h,
    KEY_I = SDLK_i,
    KEY_J = SDLK_j,
    KEY_K = SDLK_k,
    KEY_L = SDLK_l,
    KEY_M = SDLK_m,
    KEY_N = SDLK_n,
    KEY_O = SDLK_o,
    KEY_P = SDLK_p,
    KEY_Q = SDLK_q,
    KEY_R = SDLK_r,
    KEY_S = SDLK_s,
    KEY_T = SDLK_t,
    KEY_U = SDLK_u,
    KEY_V = SDLK_v,
    KEY_W = SDLK_w,
    KEY_X = SDLK_x,
    KEY_Y = SDLK_y,
    KEY_Z = SDLK_z,

    KEY_ENTER = SDLK_RETURN,
    KEY_ESCAPE = SDLK_ESCAPE,
    KEY_SPACE = SDLK_SPACE
} Keys;

bool is_key_pressed(int key) {
    if ((key > 0) && (key < MAX_KEYBOARD_KEYS)) {
        return !keyboard_state.prev_keys_pressed[key] && keyboard_state.keys_pressed[key];
    }
    return false;
}

bool is_key_down(int key) {
    if ((key > 0) && (key < MAX_KEYBOARD_KEYS)) {
        return keyboard_state.prev_keys_pressed[key] && keyboard_state.keys_pressed[key];
    }
    return false;
}

bool is_key_released(int key) {
    if ((key > 0) && (key < MAX_KEYBOARD_KEYS)) {
        return !keyboard_state.prev_keys_released[key] && keyboard_state.keys_released[key];
    }
    return false;
}

