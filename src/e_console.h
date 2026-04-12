#ifndef CONSOLE_H
#define CONSOLE_H

static struct {
    // Write console items and read from an open file.
    const char *file;
    FILE *file_handle;
    Arena *arena;
    char buffer[1024];
    Recs32 rec;
    Recs32 draw_rec;
    Keys toggle_key;
    String8 *lines;
    b32 open;
    b32 open_max;
	f32 scroll_offset;
} console = {
    .file = "data/console.txt",
    .toggle_key = KEY_F2,
    .scroll_offset = 0,
    .open = false,
    .open_max = false,
};

void console_init(void);
void console_deinit(void);
void console_update(f32 mouse_scroll);
void console_draw(Font *font);
void console_write_log(String8 log);
void console_write_log_alloc(const char *str, ...);

#endif // CONSOLE_H
