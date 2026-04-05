#include "console.h"

#define COLOR_CONSOLE  (Color){ 15, 125, 185, 240 }

#define MAX_CONSOLE_HEIGHT 300
// THIS NEEDS TO BE A DIVISOR OF THE MAX_CONSOLE_HEIGHT
#define CONSOLE_OPEN_SPEED 20

void console_init(void)
{
    console.file_handle = fopen(console.file, "w+");
    console.rec = (Recs32){.x = 0, .y = 0, .w = renderer.width, .h = 0};
}
void console_deinit(void);
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

void console_draw(void)
{
    if (console.open || console.rec.h != 0) {
        draw_recs32(console.rec, 1.1, COLOR_CONSOLE);
    }
    //draw_recs32(console.rec, 1.09, COLOR_BLACK);

}
