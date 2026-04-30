#ifndef LOG_H
#define LOG_H

typedef struct {
    String8 string;
    Color   color;
} GameLog;

static struct {
    Arena   *arena;
    GameLog *logs; // STB_ARRAY
} g_logger = {
    .arena = NULL,
    .logs = NULL,
};

void gamelogger_init(void);
void gamelogger_write_log(void);

#endif // LOG_H
