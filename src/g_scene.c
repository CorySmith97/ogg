
Scene *scene_load(Arena *arena, const char *name)
{
    Scene *s = arena_push(arena, sizeof(Scene), _Alignof(Scene), 1);
    FILE *f;
    Defer(f = fopen(name, "w"), fclose(f)) {
        s32 tile_len;
        read_bytes(f, &tile_len, sizeof(s32));
        for (int i = 0; i < tile_len; i++) {
        }

        s32 static_len;
        read_bytes(f, &static_len, sizeof(s32));
        for (int i = 0; i < static_len; i++) {
            entity_serialize(f, &s->static_entities[i]);
        }

        s32 dynamic_len;
        read_bytes(f, &dynamic_len, sizeof(s32));
        for (int i = 0; i < dynamic_len; i++) {
            entity_serialize(f, &s->dynamic_entities[i]);
        }
    }
    return NULL;
}

void scene_unload(Scene *s, const char *name)
{
}

void scene_save(Scene *s, const char *name)
{
    FILE *f = fopen(name, "w"); {
        s32 tile_len = arrlen(s->tiles);
        write_bytes(f, &tile_len, sizeof(s32));
        for (int i = 0; i < arrlen(s->tiles); i++) {
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
    } fclose(f);

}

void scene_update(Scene *s)
{
    for (int i = 0; i < arrlen(s->dynamic_entities); i++) {
        entity_update(&s->dynamic_entities[i]);
    }
}

void scene_draw(Scene *s)
{
    for (int i = 0; i < arrlen(s->tiles); i++) {
        tile_draw(&s->tiles[i]);
    }
    for (int i = 0; i < arrlen(s->static_entities); i++) {
        entity_draw(&s->static_entities[i]);
    }
    for (int i = 0; i < arrlen(s->dynamic_entities); i++) {
        entity_draw(&s->dynamic_entities[i]);
    }
}
