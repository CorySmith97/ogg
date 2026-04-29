#ifndef PLATFORM_SDL_H
#define PLATFORM_SDL_H

#define MAX_KEYBOARD 512
#define MAX_KEYS 512

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
    KEY_F1 = SDL_SCANCODE_F1,
    KEY_F2 = SDL_SCANCODE_F2,
    KEY_F3 = SDL_SCANCODE_F3,
    KEY_F4 = SDL_SCANCODE_F4,
    KEY_F5 = SDL_SCANCODE_F5,
    KEY_1 = SDL_SCANCODE_1,
    KEY_2 = SDL_SCANCODE_2,
    KEY_3 = SDL_SCANCODE_3,
    KEY_4 = SDL_SCANCODE_4,
    KEY_5 = SDL_SCANCODE_5,
    KEY_TAB = SDL_SCANCODE_TAB,
    KEY_LEFT_CONTROL  = SDL_SCANCODE_LCTRL,
    KEY_LEFT_BRACKET  = SDL_SCANCODE_LEFTBRACKET,
    KEY_RIGHT_BRACKET = SDL_SCANCODE_RIGHTBRACKET,

    KEY_BACKSPACE = SDL_SCANCODE_BACKSPACE,
    KEY_ENTER = SDL_SCANCODE_RETURN,
    KEY_DELETE = SDL_SCANCODE_DELETE,
    KEY_ESCAPE = SDL_SCANCODE_ESCAPE,
    KEY_SPACE = SDL_SCANCODE_SPACE
} Keys;

typedef enum {
    MOUSEBUTTON_LEFT = 1,
    MOUSEBUTTON_RIGHT = 3,
    MOUSEBUTTON_MIDDLE = 2,
    MOUSEBUTTON_COUNT,
} MouseCode;

typedef struct MouseState {
    f32 mouse_pos_x;
    f32 mouse_pos_y;
    f32 mouse_pos_dx;
    f32 mouse_pos_dy;
    f32 scroll_delta;
    b32 mouse_button_state[MOUSEBUTTON_COUNT];
    b32 prev_mouse_button_state[MOUSEBUTTON_COUNT];
} MouseState;


typedef struct KeyboardState {
    bool key_curr_state[MAX_KEYS];
    bool key_previous_state[MAX_KEYS];
    bool key_pressed[MAX_KEYS];
} KeyboardState;


static struct {
    SDL_Window      *window;
    SDL_GLContext    gl_ctx;
    SDL_Renderer    *renderer;
    SDL_Texture     *texture;
    uint32_t        width;
    uint32_t        height;
    u32             screen_tex;
    u32             screen_vao;
    u32             screen_prog;
    const uint8_t   *keystate;
    u8              prev_keystate[SDL_NUM_SCANCODES];
    MouseState      mouse_state;
    bool            mouse_enabled;
    char            text_input_buffer[2048];
    b32             text_input_enabled;
    struct nk_context *ui;
    char            input[1024];
    s32             input_index;
} platform_ctx = {
    .prev_keystate = {0},
    .text_input_enabled = false,
    .text_input_buffer = {0},
    .input = {0},
};


void present(void);
void platform_init(const char *name, uint32_t width, uint32_t height);
void platform_handle_events(bool *quit);
void platform_deinit(void);
void platform_present(void);
bool is_key_down(int key);
bool is_key_pressed(int key);
bool is_key_released(int key);
void set_escape_quit(bool *quit);
uint64_t get_time(void);

#endif // PLATFORM_SDL_H
