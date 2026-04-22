bool is_key_down(s32 key);
bool is_key_released(s32 key);
bool is_key_pressed_raw(s32 key);
bool is_key_down_raw(s32 key);
void on_key_down(s32 key);
void on_key_up(s32 key);
void on_mouse_moved(f32 x, f32 y, f32 dx, f32 dy);
void on_mouse_down(s32 button);
void on_mouse_up(s32 button);
void on_mouse_scroll(f32 y);
void on_text_input(char *text);
void platform_check_keystate(void);
void begin_input_frame(void);

static KeyboardState keyboard_state;
static MouseState mouse_state;

static float nk_text_width_cb(nk_handle h, float height, const char *text, int len) {
    (void)h; (void)height;
    if (len < 0) len = (int)strlen(text);
    return (float)(len * 8);
}

static struct nk_user_font nk_ui_font;

void platform_init(const char *name, u32 width, u32 height)
{
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) != SDL_FALSE)
    {
        log_error("Failed to initial Video\n");
        exit(EXIT_FAILURE);
    }

    platform_ctx.width  = width;
    platform_ctx.height = height;

    platform_ctx.window = SDL_CreateWindow(name,
                                           SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                           width, height,
                                           SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    platform_ctx.renderer = SDL_CreateRenderer(platform_ctx.window, 0, SDL_RENDERER_ACCELERATED);
    platform_ctx.texture  = SDL_CreateTexture(platform_ctx.renderer,
                                              SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING,
                                              GAME_WIDTH, GAME_HEIGHT);

    nk_ui_font.userdata = nk_handle_ptr(NULL);
    nk_ui_font.height   = 16;
    nk_ui_font.width    = nk_text_width_cb;

    platform_ctx.ui = malloc(sizeof(struct nk_context));
    nk_init_default(platform_ctx.ui, &nk_ui_font);
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
    begin_input_frame();

    struct nk_context *ctx = platform_ctx.ui;
    nk_input_begin(ctx);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
                *quit = true;
                break;
            case SDL_KEYDOWN: {
                SDL_Keycode sym = event.key.keysym.sym;
                if (sym == SDLK_LSHIFT || sym == SDLK_RSHIFT) nk_input_key(ctx, NK_KEY_SHIFT, 1);
                if (sym == SDLK_LCTRL  || sym == SDLK_RCTRL)  nk_input_key(ctx, NK_KEY_CTRL,  1);
                if (sym == SDLK_RETURN)    nk_input_key(ctx, NK_KEY_ENTER,     1);
                if (sym == SDLK_BACKSPACE) nk_input_key(ctx, NK_KEY_BACKSPACE, 1);
                if (sym == SDLK_DELETE)    nk_input_key(ctx, NK_KEY_DEL,       1);
                if (sym == SDLK_UP)        nk_input_key(ctx, NK_KEY_UP,        1);
                if (sym == SDLK_DOWN)      nk_input_key(ctx, NK_KEY_DOWN,      1);
                if (sym == SDLK_LEFT)      nk_input_key(ctx, NK_KEY_LEFT,      1);
                if (sym == SDLK_RIGHT)     nk_input_key(ctx, NK_KEY_RIGHT,     1);
                if (sym == SDLK_TAB)       nk_input_key(ctx, NK_KEY_TAB,       1);
            } break;
            case SDL_KEYUP: {
                SDL_Keycode sym = event.key.keysym.sym;
                if (sym == SDLK_LSHIFT || sym == SDLK_RSHIFT) nk_input_key(ctx, NK_KEY_SHIFT, 0);
                if (sym == SDLK_LCTRL  || sym == SDLK_RCTRL)  nk_input_key(ctx, NK_KEY_CTRL,  0);
                if (sym == SDLK_RETURN)    nk_input_key(ctx, NK_KEY_ENTER,     0);
                if (sym == SDLK_BACKSPACE) nk_input_key(ctx, NK_KEY_BACKSPACE, 0);
                if (sym == SDLK_DELETE)    nk_input_key(ctx, NK_KEY_DEL,       0);
                if (sym == SDLK_UP)        nk_input_key(ctx, NK_KEY_UP,        0);
                if (sym == SDLK_DOWN)      nk_input_key(ctx, NK_KEY_DOWN,      0);
                if (sym == SDLK_LEFT)      nk_input_key(ctx, NK_KEY_LEFT,      0);
                if (sym == SDLK_RIGHT)     nk_input_key(ctx, NK_KEY_RIGHT,     0);
                if (sym == SDLK_TAB)       nk_input_key(ctx, NK_KEY_TAB,       0);
            } break;
            case SDL_MOUSEBUTTONDOWN: {
                on_mouse_down(event.button.button);
                f32 sx = (f32)GAME_WIDTH  / (f32)platform_ctx.width;
                f32 sy = (f32)GAME_HEIGHT / (f32)platform_ctx.height;
                int bx = (int)(event.button.x * sx);
                int by = (int)(event.button.y * sy);
                if (event.button.button == SDL_BUTTON_LEFT)   nk_input_button(ctx, NK_BUTTON_LEFT,   bx, by, 1);
                if (event.button.button == SDL_BUTTON_RIGHT)  nk_input_button(ctx, NK_BUTTON_RIGHT,  bx, by, 1);
                if (event.button.button == SDL_BUTTON_MIDDLE) nk_input_button(ctx, NK_BUTTON_MIDDLE, bx, by, 1);
            } break;
            case SDL_MOUSEBUTTONUP: {
                on_mouse_up(event.button.button);
                f32 sx = (f32)GAME_WIDTH  / (f32)platform_ctx.width;
                f32 sy = (f32)GAME_HEIGHT / (f32)platform_ctx.height;
                int bx = (int)(event.button.x * sx);
                int by = (int)(event.button.y * sy);
                if (event.button.button == SDL_BUTTON_LEFT)   nk_input_button(ctx, NK_BUTTON_LEFT,   bx, by, 0);
                if (event.button.button == SDL_BUTTON_RIGHT)  nk_input_button(ctx, NK_BUTTON_RIGHT,  bx, by, 0);
                if (event.button.button == SDL_BUTTON_MIDDLE) nk_input_button(ctx, NK_BUTTON_MIDDLE, bx, by, 0);
            } break;
            case SDL_MOUSEWHEEL:
                on_mouse_scroll(event.wheel.y);
                nk_input_scroll(ctx, nk_vec2(0.0f, (float)event.wheel.y));
                break;
            case SDL_TEXTINPUT:
                if (platform_ctx.text_input_enabled)
                    on_text_input(event.text.text);
                for (int i = 0; event.text.text[i] && i < 32; i++)
                    nk_input_char(ctx, event.text.text[i]);
                break;
            case SDL_MOUSEMOTION: {
                f32 sx = (f32)GAME_WIDTH  / (f32)platform_ctx.width;
                f32 sy = (f32)GAME_HEIGHT / (f32)platform_ctx.height;
                int ui_mouse_x = (int)(event.motion.x * sx);
                int ui_mouse_y = (int)(event.motion.y * sy);
                nk_input_motion(ctx, ui_mouse_x, ui_mouse_y);
                on_mouse_moved(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
            } break;
            default:
                break;
        }
    }

    nk_input_end(ctx);


    if (platform_ctx.text_input_enabled) {
        static s32 frame = 0;
        b32 pressed = is_key_down_raw(KEY_BACKSPACE);
        if (pressed) {
            if (platform_ctx.input_index > 0 && frame >= 30) {
                platform_ctx.input_index -= 1;
                platform_ctx.input[platform_ctx.input_index] = '\0';
            }
            frame++;
        }
    }
    *quit |= is_key_pressed_raw(KEY_ESCAPE);
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

void on_text_input(char *text)
{
    s32 text_len = strlen(text);
    s32 buf_size = sizeof(platform_ctx.input); // or whatever your buffer size is

    // Bounds check before writing
    if (platform_ctx.input_index + text_len >= buf_size - 1) return;

    memcpy(platform_ctx.input + platform_ctx.input_index, text, text_len);
    platform_ctx.input_index += text_len;
    platform_ctx.input[platform_ctx.input_index] = '\0'; // keep it null-terminated
}
char *get_input_text(void)
{
    return platform_ctx.input;
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

b32 is_mouse_button_released(s32 key) {
    bool pressed = false;
    if ((key > 0) && (key < MOUSEBUTTON_COUNT)) {
        if (!mouse_state.mouse_button_state[key] 
                && mouse_state.prev_mouse_button_state[key]) {
            pressed = true;
        }
    }
    return pressed;
}

u64 get_time()
{
    return SDL_GetPerformanceCounter();
}

bool is_key_down_raw(s32 key)
{
    return platform_ctx.keystate[key];
}

bool is_key_pressed_raw(s32 key)
{
    return platform_ctx.keystate[key] && !platform_ctx.prev_keystate[key];
}

bool is_key_down(s32 key)
{
    if (platform_ctx.text_input_enabled) return false;
    return platform_ctx.keystate[key];
}

bool is_key_pressed(s32 key)
{
    if (platform_ctx.text_input_enabled) return false;
    return platform_ctx.keystate[key] && !platform_ctx.prev_keystate[key];
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
    *quit = is_key_pressed_raw(KEY_ESCAPE);
}

void set_mouse_toggle_key(s32 key)
{
    if (is_key_down(key)) {
        platform_ctx.mouse_enabled = !platform_ctx.mouse_enabled;
    }
}
void begin_input_frame(void)
{
    platform_ctx.mouse_state.scroll_delta = 0;
}

void on_mouse_scroll(f32 y)
{
    platform_ctx.mouse_state.scroll_delta += y;
}

f32 get_mouse_scroll(void)
{
    return platform_ctx.mouse_state.scroll_delta;
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

// TODO
// add copy paste with clipboard.
//
// add input reset
