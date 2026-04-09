#ifndef CONSOLE_H
#define CONSOLE_H

static struct {
    // Write console items and read from an open file.
    const char *file;
    FILE *file_handle;
    Recs32 rec;
    Recs32 draw_rec;
    Keys toggle_key;
    const char **lines;
    b32 open;
	f32 scroll_offset;
} console = {
    .file = "data/console.txt",
    .toggle_key = KEY_F2,
    .open = false,
};

void console_init(void);
void console_deinit(void);
void console_update(void);
void console_draw(Font *font);

#endif // CONSOLE_H
