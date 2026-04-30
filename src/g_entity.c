static void update_shopkeeper(Entity *e)
{
	//static f32 angle = 0;
	//e->rotation = rotation_y(angle);
	//angle += 0.01;
}

void entity_init(void)
{
    global_entities[ENTITY_TAG_SHOPKEEPER] = (Entity){
        .update_fn = update_shopkeeper,
    };
}

f32 angle_diff(f32 a, f32 b) {
    f32 d = b - a;
    while (d >  M_PI) d -= 2.0f * M_PI;
    while (d < -M_PI) d += 2.0f * M_PI;
    return d;
}

void entity_anim_init(Entity *e, const char *skel, const char *skin, const char *anim)
{
    if (!e->model) return;
    skin_load(skin, e->model);
    e->anim_data = anim_data_load(skel, anim);
    anim_state_init(&e->anim, e->anim_data, e->model);
}

void entity_update(Entity *e)
{
    e->position = v3f_add(e->position, v3f_scale(v3f_sub(e->target, e->position), 0.1));
    V3f dir = v3f_sub(e->target, e->position);
    if (!v3f_equal(dir, v3f(0,0,0))) {
        e->rotation = rotation_y(atan2f(dir.x, -dir.z));
    }

    if (e->anim_data)
        anim_update(&e->anim, renderer.dt * 1000.0f);

    e->update_fn(e);
}

void entity_draw(Entity *e)
{
    if (e->anim_data)
        anim_apply(&e->anim, e->model);
    draw_model(e->model, e->position, e->rotation, e->hit);
}

RayCollision entity_mouse_ray_collision(Entity *e, Ray mouse_ray)
{
    RayCollision collision = {0};
    for (s32 i = 0; i < arrlen(e->model->vertices); i += 3) {
        Vertex v1 = e->model->vertices[i];
        Vertex v2 = e->model->vertices[i + 1];
        Vertex v3 = e->model->vertices[i + 2];

        V3f p1 = v3f_mul_mat3(v1.position, e->rotation);
        V3f p2 = v3f_mul_mat3(v2.position, e->rotation);
        V3f p3 = v3f_mul_mat3(v3.position, e->rotation);
        p1 = v3f_add(p1, e->position);
        p2 = v3f_add(p2, e->position);
        p3 = v3f_add(p3, e->position);

        RayCollision tri_hit = get_ray_collision_triangle(mouse_ray, p1, p2, p3);
        if (tri_hit.hit) {
            if ((!collision.hit) || (collision.distance > tri_hit.distance)) collision = tri_hit;
        }
    }

    return collision;
}


// ----------------------------------------------------------------
// Serializer
// ----------------------------------------------------------------

bool entity_serialize(FILE *f, const Entity *e) {
    bool ok = true;

    // POD fields
    ok &= write_bytes(f, &e->tag,              sizeof(e->tag));
    // model            -- SKIPPED
    ok &= write_string(f,  e->model_tag);           // const char *
    ok &= write_bytes(f, &e->position,         sizeof(e->position));
    ok &= write_bytes(f, &e->target,           sizeof(e->target));
    ok &= write_bytes(f, &e->rotation,         sizeof(e->rotation));
    ok &= write_bytes(f, &e->aabb,             sizeof(e->aabb));
    ok &= write_bytes(f, &e->hit,              sizeof(e->hit));
    ok &= write_bytes(f, &e->yaw,              sizeof(e->yaw));
    ok &= write_bytes(f, &e->update_disabled,  sizeof(e->update_disabled));
    ok &= write_bytes(f, &e->selected_player,  sizeof(e->selected_player));
    ok &= write_bytes(f, &e->race,             sizeof(e->race));
    ok &= write_bytes(f, &e->base_class,       sizeof(e->base_class));
    ok &= write_bytes(f, &e->attributes,       sizeof(e->attributes));
    ok &= write_bytes(f, &e->level,            sizeof(e->level));
    ok &= write_bytes(f, &e->movement_speed,   sizeof(e->movement_speed));
    ok &= write_bytes(f, &e->spellcaster_lvl,  sizeof(e->spellcaster_lvl));
    // update_fn        -- SKIPPED

    fclose(f);
    return ok;
}

// ----------------------------------------------------------------
// Deserializer
// ----------------------------------------------------------------

// model_tag_buf / buf_size: caller-owned buffer for the model_tag string.
// After loading, e->model and e->update_fn are set to NULL.
bool entity_deserialize(Arena *arena, Entity *e, const char *path,
                        char *model_tag_buf, size_t model_tag_buf_size) {
    FILE *f = fopen(path, "rb");
    if (!f) return false;

    bool ok = true;

    ok &= read_bytes(f, &e->tag,             sizeof(e->tag));
    e->model = NULL;                                         // not serialized
    ok &= read_string(f, model_tag_buf, model_tag_buf_size);
    e->model_tag = ok ? model_tag_buf : NULL;
    ok &= read_bytes(f, &e->position,        sizeof(e->position));
    ok &= read_bytes(f, &e->target,          sizeof(e->target));
    ok &= read_bytes(f, &e->rotation,        sizeof(e->rotation));
    ok &= read_bytes(f, &e->aabb,            sizeof(e->aabb));
    ok &= read_bytes(f, &e->hit,             sizeof(e->hit));
    ok &= read_bytes(f, &e->yaw,             sizeof(e->yaw));
    ok &= read_bytes(f, &e->update_disabled, sizeof(e->update_disabled));
    ok &= read_bytes(f, &e->selected_player, sizeof(e->selected_player));
    ok &= read_bytes(f, &e->race,            sizeof(e->race));
    ok &= read_bytes(f, &e->base_class,      sizeof(e->base_class));
    ok &= read_bytes(f, &e->attributes,      sizeof(e->attributes));
    ok &= read_bytes(f, &e->level,           sizeof(e->level));
    ok &= read_bytes(f, &e->movement_speed,  sizeof(e->movement_speed));
    ok &= read_bytes(f, &e->spellcaster_lvl, sizeof(e->spellcaster_lvl));
    e->update_fn = NULL;                                     // not serialized

    fclose(f);
    return ok;
}
