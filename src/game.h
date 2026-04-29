#pragma once

typedef enum {
    GAME_STATE_EDITOR,
    GAME_STATE_MENU,
    GAME_STATE_GAMEPLAY,
    GAME_STATE_PAUSE,
    GAME_STATE_MODEL_EDITOR,
    GAME_STATE_COUNT,
} GameState;

void game_run(void);
void game_init(void);
void game_frame(void);
void game_deinit(void);

#include "g_pathfinder.h"
#include "g_entity.h"
#include "g_tile.h"
#include "g_scene.h"
#include "g_algorithms.h"
