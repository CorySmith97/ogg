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

static int text_width(mu_Font font, const char *text, int len) {
  if (len == -1) { len = strlen(text); }
  return len * 16;
}

static int text_height(mu_Font font) {
  return 16;
}

void platform_init(const char *name, u32 width, u32 height)
{
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) != SDL_FALSE)
    {
        log_error("Failed to initial Video\n");
        exit(EXIT_FAILURE);
    }

    platform_ctx.width = width;
    platform_ctx.height = height;
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


    // TODO these functions need to be implemented
    platform_ctx.ui->text_width  = text_width;
    platform_ctx.ui->text_height = text_height;
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
    s32 b, c;
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
            case SDL_MOUSEWHEEL:
                on_mouse_scroll(event.wheel.y);
                mu_input_scroll(platform_ctx.ui, 0, event.wheel.y * -30); 
                                 break;
            case SDL_TEXTINPUT: 
                if (platform_ctx.text_input_enabled) {
                    on_text_input(event.text.text);
                    //mu_input_text(platform_ctx.ui, event.text.text); 
                }
                break;
            case SDL_MOUSEMOTION: {
                                      f32 sx = (f32)GAME_WIDTH  / (f32)platform_ctx.width;
                                      f32 sy = (f32)GAME_HEIGHT / (f32)platform_ctx.height;

                                      int ui_mouse_x = (int)(event.motion.x * sx);
                                      int ui_mouse_y = (int)(event.motion.y * sy);
                                      mu_input_mousemove(platform_ctx.ui, ui_mouse_x, ui_mouse_y);
                                      on_mouse_moved(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
                                  } break;
            default:
                break;
        }
    }

    if (platform_ctx.text_input_enabled) {
        b32 pressed = is_key_pressed_raw(KEY_BACKSPACE);
        if (pressed) {
            if (platform_ctx.input_index > 0) {
                platform_ctx.input_index -= 1;
                platform_ctx.input[platform_ctx.input_index] = '\0';
            }
        }
    }
    *quit = is_key_pressed_raw(KEY_ESCAPE);
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

