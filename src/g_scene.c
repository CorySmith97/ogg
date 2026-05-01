typedef struct {
    char *key;
    Scene *value;
} Scene_KV;

Scene_KV *scenes = NULL;

void console_change_scene(String8 param)
{
    //gs.loaded_scene = scene_load(gs.arena, str8_to_cstring(param));
}

/* Scene *scene_load(Arena *arena, String8 name)
{
    
} */

Scene *scene_new(Arena *arena, const char *name)
{
    Scene *s = arena_push_struct(arena, Scene);
    shput(scenes, name, s);
    return s;
}

Scene *scene_load(Arena *arena, const char *name)
{
    Scene *s = arena_push_struct(arena, Scene);
    //String8 open_and_read_entire_file(arena, name);
    FILE *f;
    Defer(f = fopen(name, "w"), fclose(f)) {
        s32 tile_len;
        read_bytes(f, &tile_len, sizeof(s32));
        for (int i = 0; i < tile_len; i++) {
            //tile_deserialize(f, &s->tiles[i]);
        }

        s32 static_len;
        read_bytes(f, &static_len, sizeof(s32));
        for (int i = 0; i < static_len; i++) {
            //entity_deserialize(f, &s->static_entities[i]);
        }

        s32 dynamic_len;
        read_bytes(f, &dynamic_len, sizeof(s32));
        for (int i = 0; i < dynamic_len; i++) {
            //entity_deserialize(f, &s->dynamic_entities[i]);
        }
    }

    return s;
}

void scene_unload(Scene *s, const char *name)
{
}

void scene_save(Scene *s, const char *name)
{
    FILE *f;
    Defer(f = fopen(name, "r"), fclose(f)) {
        s32 tile_len = arrlen(s->tiles);
        write_bytes(f, &tile_len, sizeof(s32));
        for (int i = 0; i < arrlen(s->tiles); i++) {
            tile_serialize(f, &s->tiles[i]);
        }
        s32 static_len = arrlen(s->static_entities);
        write_bytes(f, &static_len, sizeof(s32));
        for (int i = 0; i < arrlen(s->static_entities); i++) {
            entity_serialize(f, &s->static_entities[i]);
        }
        s32 dynamic_len = arrlen(s->dynamic_entities);
        write_bytes(f, &dynamic_len, sizeof(s32));
        for (int i = 0; i < arrlen(s->dynamic_entities); i++) {
            entity_serialize(f, &s->dynamic_entities[i]);
        }
    }

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
                Entity *e = &s->dynamic_entities[gs.player_index];
                e->target = s->tiles[i].position;
            }
        }
    }
    for (int i = 0; i < arrlen(s->dynamic_entities); i++) {
        entity_update(&s->dynamic_entities[i]);
    }
}

void scene_draw(Scene *s)
{
    s32 tile_len = arrlen(s->tiles);
    for (s32 i = 0; i < tile_len; i++) {
        tile_draw(&s->tiles[i]);
    }
    for (int i = 0; i < arrlen(s->static_entities); i++) {
        entity_draw(&s->static_entities[i]);
    }
    for (int i = 0; i < arrlen(s->dynamic_entities); i++) {
        entity_draw(&s->dynamic_entities[i]);
    }
}
