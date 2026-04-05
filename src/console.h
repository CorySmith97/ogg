#ifndef CONSOLE_H
#define CONSOLE_H

static struct {
    // Write console items and read from an open file.
    const char *file;
    FILE *file_handle;
    Recs32 rec;
    Keys toggle_key;
    char **lines;
    f32 open_percentage;
    b32 open;
} console = {
    .file = "data/console.txt",
    .toggle_key = KEY_F2,
    .open = false,
};

void console_init(void);
void console_deinit(void);
void console_update(void);
void console_draw(void);

#endif // CONSOLE_H
