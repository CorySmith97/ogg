#ifndef SCENE_H
#define SCENE_H

typedef struct {
    String8 name;
    // Gameplay
    Entity_Manager *manager;
    Entity *dynamic_entities;
    Entity *static_entities;
    Tile *tiles;
} Scene;

void console_change_scene(String8 param);
void console_new_scene(String8 param);

Scene *scene_new(Arena *arena, const char *name, Entity_Manager *manager);
Scene *scene_load(Arena *arena, const char *name);
void   scene_unload(Scene *s, const char *name);
void   scene_save(Scene *s, const char *name);
void   scene_update(Scene *s);
void   scene_draw(Scene *s);

#endif // SCENE_H
