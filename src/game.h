#ifndef GAME_H
#define GAME_H

#define GAME_WIDTH    1024
#define GAME_HEIGHT   576
#define SCREEN_WIDTH  1980
#define SCREEN_HEIGHT 1080
#define GAME_NAME     "OGG"

typedef enum {
    GAME_STATE_EDITOR,
    GAME_STATE_MENU,
    GAME_STATE_GAMEPLAY,
    GAME_STATE_PAUSE,
    GAME_STATE_COUNT,
} GameState;

void game_run(void);
void game_init(void);
void game_frame(void);
void game_deinit(void);

#endif // GAME_H
