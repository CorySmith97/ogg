#ifndef SCENE_H
#define SCENE_H

typedef struct {
    // Gameplay
    Entity *dynamic_entities;
    Entity *static_entities;
    Tile *tiles;
} Scene;

Scene *scene_load(Arena *arena, const char *name);
void   scene_unload(Scene *s, const char *name);
void   scene_save(Scene *s, const char *name);
void   scene_update(Scene *s);
void   scene_draw(Scene *s);

#endif // SCENE_H
