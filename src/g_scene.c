typedef struct {
    char *key;
    Scene *value;
} Scene_KV;

Scene_KV *scenes = NULL;

void console_change_scene(String8 param)
{
    editor.scene = NULL;
    char *name = str8_to_cstring(console.arena, param);
    Scene *s = scene_load(editor.arena, name);
    editor_set_scene(s);

    console_write_log_alloc("Opened scene: %s", name);
}

/* Scene *scene_load(Arena *arena, String8 name)
{
    
} */

Scene *scene_new(Arena *arena, const char *name, Entity_Manager *manager)
{
    Scene *s = arena_push_struct(arena, Scene);
    s->name = str8_fmt_alloc("%s", name);
    s->manager = manager;
    shput(scenes, name, s);
    return s;
}

Scene *scene_load(Arena *arena, const char *name)
{
    Scene *s = arena_push_struct(arena, Scene);
    char *buf = NULL;
    asprintf(&buf, "data/level/%s.lvl", name);
    console_write_log_alloc("Loading scene: %s", buf);
    //String8 open_and_read_entire_file(arena, name);
    FILE *f;
    Defer(f = fopen(buf, "r"), fclose(f)) {
        if (!f) continue;
        s32 tile_len;
        read_bytes(f, &tile_len, sizeof(s32));
        arrsetlen(s->tiles, tile_len);
        for (int i = 0; i < tile_len; i++) {
            Tile *t = &s->tiles[i];
            b32 ok = tile_deserialize(f, t);
            if (!ok)
                console_write_log_alloc("Failed to deserialize tile %d", i);
        }

        s32 dynamic_len;
        read_bytes(f, &dynamic_len, sizeof(s32));
        s->manager->current_len = dynamic_len;
        for (int i = 0; i < dynamic_len; i++) {
            Entity *e = &s->manager->entities[i];
            b32 ok = entity_deserialize(editor.arena, f, e);
            if (!ok)
                console_write_log_alloc("Failed to deserialize entity %d", i);
        }
    }

    return s;
}

void scene_unload(Scene *s, const char *name)
{
}

void scene_save(Scene *s, const char *name)
{
    char *buf = NULL;
    asprintf(&buf, "data/level/%s.lvl", s->name.data);
    FILE *f;
    Defer(f = fopen(buf, "w"), fclose(f)) {
        s32 tile_len = arrlen(s->tiles);
        write_bytes(f, &tile_len, sizeof(s32));
        for (int i = 0; i < arrlen(s->tiles); i++) {
            b32 ok = tile_serialize(f, &s->tiles[i]);
            if (!ok)
                console_write_log_alloc("Failed to serialize tile %d", i);
        }
        s32 dynamic_len = s->manager->current_len;
        write_bytes(f, &dynamic_len, sizeof(s32));
        for (int i = 0; i < dynamic_len; i++) {
            b32 ok = entity_serialize(f, &s->manager->entities[i]);
            if (!ok)
                console_write_log_alloc("Failed to serialize tile %d", i);
        }
    }
    free(buf);

}

void scene_update(Scene *s)
{
    f32 mouse_scroll = get_mouse_scroll();
    V2f mouse_pos = get_mouse_pos();
    V2f mouse_delta = get_mouse_delta();
    Ray mouse_ray   = get_mouse_ray(&gs.camera, mouse_pos);

    if (console.open) return;

    handle_camera_gameplay(mouse_delta);

    if (is_mouse_button_pressed(MOUSEBUTTON_LEFT)) {
        for (s32 i = 0; i < arrlen(s->tiles); i++) {
            RayCollision collision = tile_mouse_ray_collision(&s->tiles[i], mouse_ray);
            if (collision.hit) {
                Entity *e = &s->manager->entities[s->manager->player_index];
                e->target = s->tiles[i].position;
            }
        }
    }
    for (int i = 1; i < s->manager->current_len; i++) {
        entity_update(&s->manager->entities[i]);
    }
}

void scene_draw(Scene *s)
{
    s32 tile_len = arrlen(s->tiles);
    for (s32 i = 0; i < tile_len; i++) {
        tile_draw(&s->tiles[i]);
    }
    for (int i = 1; i < s->manager->current_len; i++) {
        entity_draw(&s->manager->entities[i]);
    }
}
