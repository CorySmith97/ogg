#include "platform_sdl.h"

bool is_key_down(s32 key);
bool is_key_released(s32 key);
void on_key_down(s32 key);
void on_key_up(s32 key);
void on_mouse_moved(f32 x, f32 y, f32 dx, f32 dy);
void on_mouse_down(s32 button); 
void on_mouse_up(s32 button);
void on_mouse_scroll(f32 y);
void platform_check_keystate(void);

static KeyboardState keyboard_state;
static MouseState mouse_state;

void platform_init(const char *name, u32 width, u32 height)
{
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) != SDL_FALSE)
    {
        log_error("Failed to initial Video\n");
        exit(EXIT_FAILURE);
    }

    platform_ctx.window = SDL_CreateWindow(name,
                                           SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                           width, height,
                                           SDL_WINDOW_SHOWN);

    platform_ctx.renderer = SDL_CreateRenderer(platform_ctx.window, 0, SDL_RENDERER_ACCELERATED);
    platform_ctx.texture = SDL_CreateTexture(platform_ctx.renderer,
                                             SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING,
                                             GAME_WIDTH, GAME_HEIGHT);

}

void platform_handle_events(bool *quit)
{
    if (platform_ctx.mouse_enabled) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    } else {
        // TODO add a way to lock mouse to screen.
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    platform_check_keystate();
    s32 b, c;
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
            case SDL_MOUSEBUTTONDOWN:
                on_mouse_down(event.button.button);
                break;
            case SDL_MOUSEBUTTONUP:
                on_mouse_up(event.button.button);
                break;
            case SDL_MOUSEWHEEL:
                on_mouse_scroll(event.wheel.y);
                break;
            case SDL_MOUSEMOTION:
                on_mouse_moved(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
                break;
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
    void *texpixels;
    s32 pitch;
    SDL_LockTexture(platform_ctx.texture, NULL, &texpixels, &pitch);
    memcpy(texpixels, renderer.pixels, renderer.width * renderer.height * sizeof(u32));
    SDL_UnlockTexture(platform_ctx.texture);

    SDL_RenderClear(platform_ctx.renderer);
    SDL_RenderCopy(platform_ctx.renderer, platform_ctx.texture, NULL, NULL);
    SDL_RenderPresent(platform_ctx.renderer);
}


bool is_mouse_button_down(s32 key) {
    bool pressed = false;
    if ((key > 0) && (key < MOUSEBUTTON_COUNT)) {
        if (mouse_state.mouse_button_state[key]) {
            pressed = true;
        }
    }
    return pressed;
}

b32 is_mouse_button_pressed(s32 key) {
    bool pressed = false;
    if ((key > 0) && (key < MOUSEBUTTON_COUNT)) {
        if (mouse_state.mouse_button_state[key] 
                && !mouse_state.prev_mouse_button_state[key]) {
            pressed = true;
        }
    }
    return pressed;
}

u64 get_time()
{
    return SDL_GetPerformanceCounter();
}

void on_key_down(s32 key) 
{
    if (keyboard_state.key_curr_state[key] == true) {
        keyboard_state.key_previous_state[key] = true;
    }
    keyboard_state.key_curr_state[key] = true;
}

void on_key_up(s32 key) 
{ 
    keyboard_state.key_curr_state[key] = false;
}

void on_mouse_down(s32 button) 
{
    mouse_state.mouse_button_state[button] = true;
}

void on_mouse_up(s32 button) 
{ 
    mouse_state.mouse_button_state[button] = false;
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

void on_mouse_moved(f32 x, f32 y, f32 dx, f32 dy) 
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

void set_mouse_toggle_key(s32 key)
{
    if (is_key_down(key)) {
        platform_ctx.mouse_enabled = !platform_ctx.mouse_enabled;
    }
}
        
void on_mouse_scroll(f32 y)
{
    platform_ctx.mouse_state.scroll_delta = y;
}

f32 get_mouse_scroll()
{
    f32 ret = platform_ctx.mouse_state.scroll_delta;
    platform_ctx.mouse_state.scroll_delta = 0;
    return ret;

}

void platform_check_keystate(void)
{

    memcpy(mouse_state.prev_mouse_button_state,
            mouse_state.mouse_button_state,
            sizeof(mouse_state.mouse_button_state));

    if (platform_ctx.keystate != NULL)
        memcpy(platform_ctx.prev_keystate,
                platform_ctx.keystate,
                SDL_NUM_SCANCODES);
    SDL_PumpEvents();

    platform_ctx.keystate = SDL_GetKeyboardState(NULL);
}

bool is_key_down(s32 key)
{
    return platform_ctx.keystate[key];
}

bool is_key_pressed(s32 key)
{
    return platform_ctx.keystate[key] && !platform_ctx.prev_keystate[key];
}

bool is_key_released(s32 key)
{
    return !platform_ctx.keystate[key] && platform_ctx.prev_keystate[key];
}
