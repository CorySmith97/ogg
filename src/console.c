#include "console.h"

#define COLOR_CONSOLE  (Color){ 0, 0, 0, 240 }
#define COLOR_CONSOLE_TEXT  (Color){ 185, 185, 185, 255 }
#define COLOR_CONSOLE_INPUT  (Color){ 15, 185, 185, 240 }

#define MAX_CONSOLE_HEIGHT 300
// THIS NEEDS TO BE A DIVISOR OF THE MAX_CONSOLE_HEIGHT
#define CONSOLE_OPEN_SPEED 20

void console_init(void)
{
    console.file_handle = fopen(console.file, "w+");
    console.rec = (Recs32){.x = 0, .y = 0, .w = renderer.width, .h = 0};
}

void console_deinit(void)
{
}

void console_update(void)
{
    if (console.open && console.rec.h != MAX_CONSOLE_HEIGHT) {
        console.rec.h += CONSOLE_OPEN_SPEED;
    }
    if (!console.open && console.rec.h != 0) {
        console.rec.h -= CONSOLE_OPEN_SPEED;
    }
    if (is_key_pressed(console.toggle_key)) {
        console.open = !console.open;
    }
}

void console_write_log(const char *log)
{
    arrput(console.lines, log);
}

void console_draw(Font *font)
{
    f32 text_size = 20;
    if (console.open || console.rec.h != 0) {
        draw_recs32(console.rec, 1.1, COLOR_CONSOLE);
        for (size_t i = 0; i < (size_t)arrlen(console.lines); i++) {
            draw_text(font, console.lines[i], v2i(3, (s32)console.rec.h - ((i + 1) * text_size)), text_size, COLOR_CONSOLE_TEXT);
        }
        draw_recs32((Recs32){.x = 0, .y = console.rec.h, .w = console.rec.w, .h = text_size}, 1.09, COLOR_CONSOLE_INPUT);
    }

}
