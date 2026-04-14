
#define COLOR_CONSOLE  (Color){ 0, 0, 0, 240 }
#define COLOR_CONSOLE_TEXT  (Color){ 185, 185, 185, 255 }
#define COLOR_CONSOLE_INPUT  (Color){ 15, 185, 185, 240 }
#define COLOR_CONSOLE_INPUT_TEXT  (Color){ 0, 0, 0, 255 }

#define MAX_CONSOLE_HEIGHT 500
#define MIN_CONSOLE_HEIGHT 200
// THIS NEEDS TO BE A DIVISOR OF THE MAX_CONSOLE_HEIGHT
#define CONSOLE_OPEN_SPEED 50

void test(String8 params)
{
    console_write_log(params);
}

void console_register_command(const char *command, console_command_fn *fn)
{
    shput(console.commands, command, fn);
}

void console_init(void)
{
    console.arena = arena_alloc();
    console.file_handle = fopen(console.file, "w+");
    console.rec = (Recs32){.x = 0, .y = 0, .w = renderer.width, .h = 0};
    console_register_command("test", test);
    console_register_command("mload", model_editor_load_model);
}

void console_deinit(void)
{
    arena_release(console.arena);
}

void console_update(f32 mouse_scroll)
{
    if (console.open && console.rec.h != MIN_CONSOLE_HEIGHT) {
        console.rec.h += CONSOLE_OPEN_SPEED;
    }
    if (console.open && console.rec.h == MIN_CONSOLE_HEIGHT) {
        platform_ctx.text_input_enabled = true;
        if (is_key_pressed(KEY_ENTER)) {
            String8 str;
            u32 len = strlen(platform_ctx.input);
            str.data = arena_push(console.arena, len, 1, 0);
            str.data = memcpy(str.data, platform_ctx.input, len);
            str.len = len;
            if (str.len > 0) {
                String8 command_str = str8_split(str, ' ');
                if (command_str.data != NULL) {

                    console_command_fn fn = shget(console.commands,str8_to_cstring(console.arena, command_str));
                    fn((String8){.data = &str.data[command_str.len+1], .len = str.len-command_str.len-1});
                    platform_ctx.input_index = 0;
                    memset(platform_ctx.input, 0, 1024);
                }
            }
        }
    }
    if (!console.open && console.rec.h == 0) {
        platform_ctx.text_input_enabled = false;
    }
    if (!console.open && console.rec.h != 0) {
        console.rec.h -= CONSOLE_OPEN_SPEED;
    }
    if (is_key_pressed_raw(console.toggle_key)) {
        console.open = !console.open;
    }
    if (console.open && mouse_scroll != 0) {
        console.scroll_offset += mouse_scroll;
    }
}

void console_write_log(String8 log)
{
    arrput(console.lines, log);
    console.scroll_offset = 0;
}

void console_draw(Font *font)
{
    if (!(console.open || console.rec.h != 0)) {
        return;
    }

    f32 text_size = 16.0f;
    s32 total_lines = (s32)arrlen(console.lines);

    // reserve one row for input
    s32 visible_lines = (s32)(console.rec.h / text_size) - 1;
    if (visible_lines < 0) visible_lines = 0;

    s32 max_scroll = total_lines - visible_lines;
    if (max_scroll < 0) max_scroll = 0;

    if (console.scroll_offset < 0) console.scroll_offset = 0;
    if (console.scroll_offset > max_scroll) console.scroll_offset = max_scroll;

    s32 start = total_lines - visible_lines - console.scroll_offset;
    if (start < 0) start = 0;

    s32 end = start + visible_lines;
    if (end > total_lines) end = total_lines;

    s32 drawn_lines = end - start;

    draw_recs32(console.rec, 0.9f, COLOR_CONSOLE);

    for (s32 i = start; i < end; i++) {
        s32 row = i - start; // 0..drawn_lines-1
        s32 y = (s32)(console.rec.h - text_size * (drawn_lines - row));
        draw_string8(font, console.lines[i], v2i(3, y), text_size, COLOR_CONSOLE_TEXT);
    }

    draw_recs32((Recs32){
        .x = 0,
        .y = console.rec.h,
        .w = console.rec.w,
        .h = text_size,
    }, 1.09f, COLOR_CONSOLE_INPUT);

    char *p = get_input_text();
    draw_text(font, p, v2i(3, console.rec.h), text_size, COLOR_CONSOLE_INPUT_TEXT);
}

void console_write_log_alloc(const char *fmt, ...)
{
    String8 result = {0};
    if (!fmt) return;

    va_list args;
    va_start(args, fmt);
    va_list args2;
    va_copy(args2, args);

    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (n < 0) {
        va_end(args2);
        return;
    }

    char *buf = arena_push(console.arena, n + 1, 8, 0);
    if (!buf) {
        va_end(args2);
        return;
    }

    vsnprintf(buf, (size_t)n + 1, fmt, args2);
    va_end(args2);

    result.data = (u8 *)buf;
    result.len  = (u32)n;
    arrput(console.lines, result);
}
