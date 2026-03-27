#ifndef GAME_H
#define GAME_H

#define GAME_WIDTH    960
#define GAME_HEIGHT   704
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720
#define GAME_NAME     "OGG"

void game_run(void);
void game_init(void);
void game_frame(void);
void game_deinit(void);

#endif // GAME_H
