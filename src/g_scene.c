
Scene *scene_load(const char *name)
{
    return NULL;
}

void scene_unload(Scene *s, const char *name)
{
}

void scene_save(Scene *s, const char *name)
{
    FILE *f = fopen(name, "w"); {
        for (int i = 0; i < arrlen(s->tiles); i++) {
        }
        for (int i = 0; i < arrlen(s->static_entities); i++) {
        }
        for (int i = 0; i < arrlen(s->dynamic_entities); i++) {
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
