#include "platform_sdl.h"

bool is_key_down(int key);
bool is_key_released(int key);
void on_key_down(int key);
void on_key_up(int key);
void on_mouse_moved(float x, float y, float dx, float dy);
void on_mouse_down(int button); 
void on_mouse_up(int button);

#define MAX_KEYS 512

typedef struct KeyboardState {
    bool key_curr_state[MAX_KEYS];
    bool key_previous_state[MAX_KEYS];
    bool key_pressed[MAX_KEYS];
} KeyboardState;

static KeyboardState keyboard_state;
static MouseState mouse_state;

static const char button_map[256] = {
  [ SDL_BUTTON_LEFT   & 0xff ] =  MU_MOUSE_LEFT,
  [ SDL_BUTTON_RIGHT  & 0xff ] =  MU_MOUSE_RIGHT,
  [ SDL_BUTTON_MIDDLE & 0xff ] =  MU_MOUSE_MIDDLE,
};

static const char key_map[256] = {
  [ SDLK_LSHIFT       & 0xff ] = MU_KEY_SHIFT,
  [ SDLK_RSHIFT       & 0xff ] = MU_KEY_SHIFT,
  [ SDLK_LCTRL        & 0xff ] = MU_KEY_CTRL,
  [ SDLK_RCTRL        & 0xff ] = MU_KEY_CTRL,
  [ SDLK_LALT         & 0xff ] = MU_KEY_ALT,
  [ SDLK_RALT         & 0xff ] = MU_KEY_ALT,
  [ SDLK_RETURN       & 0xff ] = MU_KEY_RETURN,
  [ SDLK_BACKSPACE    & 0xff ] = MU_KEY_BACKSPACE,
};

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

    platform_ctx.renderer = SDL_CreateRenderer(platform_ctx.window, 0, SDL_RENDERER_ACCELERATED);
    platform_ctx.texture = SDL_CreateTexture(platform_ctx.renderer,
                                             SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING,
                                             GAME_WIDTH, GAME_HEIGHT);

    platform_ctx.ui = malloc(sizeof(mu_Context));
    mu_init(platform_ctx.ui);

/* static int text_width(mu_Font font, const char *text, int len) {
  if (len == -1) { len = strlen(text); }
  return r_get_text_width(text, len);
}

static int text_height(mu_Font font) {
  return r_get_text_height();
} */

    // TODO these functions need to be implemented
    //platform_ctx.ui->text_width = 8;
    //platform_ctx.ui->text_height = 16;
}

void platform_handle_events(bool *quit)
{
    if (platform_ctx.mouse_enabled) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    } else {
        // TODO add a way to lock mouse to screen.
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
    int b, c;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
                *quit = true;
                break;
            case SDL_KEYDOWN:
                c = key_map[event.key.keysym.sym & 0xff];
                if (c && event.type == SDL_KEYDOWN) { mu_input_keydown(platform_ctx.ui, c); }
                //on_key_down(event.key.keysym.scancode);
                break;
            case SDL_KEYUP:
                c = key_map[event.key.keysym.sym & 0xff];
                if (c && event.type ==   SDL_KEYUP) { mu_input_keyup(platform_ctx.ui, c);   }
                //on_key_up(event.key.keysym.scancode);
                break;
            case SDL_MOUSEBUTTONDOWN:
                on_mouse_down(event.button.button);
                b = button_map[event.button.button & 0xff];
                if (b && event.type == SDL_MOUSEBUTTONDOWN) { mu_input_mousedown(platform_ctx.ui, event.button.x, event.button.y, b); }
                break;
            case SDL_MOUSEBUTTONUP:
                b = button_map[event.button.button & 0xff];
                if (b && event.type ==   SDL_MOUSEBUTTONUP) { mu_input_mouseup(platform_ctx.ui, event.button.x, event.button.y, b);   }
                on_mouse_up(event.button.button);
                break;
            case SDL_MOUSEWHEEL: mu_input_scroll(platform_ctx.ui, 0, event.wheel.y * -30); break;
            case SDL_TEXTINPUT: mu_input_text(platform_ctx.ui, event.text.text); break;
            case SDL_MOUSEMOTION:
                mu_input_mousemove(platform_ctx.ui, event.motion.x, event.motion.y);
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

bool is_mouse_button_down(int key) {
    bool pressed = false;
    if ((key > 0) && (key < MOUSEBUTTON_COUNT)) {
        if (mouse_state.mouse_button_state[key]) {
            pressed = true;
        }
    }
    return pressed;
}

uint64_t get_time()
{
    return SDL_GetPerformanceCounter();
}

bool is_key_released(int key) 
{
    bool pressed = false;
    if ((key > 0) && (key < MAX_KEYS)) {
        if ((keyboard_state.key_previous_state[key] == true) && (keyboard_state.key_curr_state[key] == false))
        pressed = true;
    }
    return pressed;
}

void on_key_down(int key) 
{
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
    mouse_state.mouse_button_state[button] = true;
}

void on_mouse_up(int button) 
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

void set_mouse_toggle_key(int key)
{
    if (is_key_down(key)) {
        platform_ctx.mouse_enabled = !platform_ctx.mouse_enabled;
    }
}
        
