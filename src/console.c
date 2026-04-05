#include "console.h"


void console_init(void)
{
    console.file_handle = fopen(console.file, "w+");
    console.rec = (Recs32){.x = 0, .y = 0, .w = renderer.width, .h = 300};
}
void console_deinit(void);
void console_update(void)
{
    if (is_key_pressed(console.toggle_key)) {
        console.open = !console.open;
    }
}

void console_draw(void)
{
    if (console.open) {
        draw_recs32(console.rec,1.1, COLOR_TRANS);
    }
    //draw_recs32(console.rec, 1.09, COLOR_BLACK);

}
