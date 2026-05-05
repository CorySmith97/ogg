Entity *get_new_entity(Entity_Manager *manager)
{
    Entity *e = &manager->entities[manager->current_len];
    manager->current_len += 1;
    return e;
}

Entity *get_selected_entity(Entity_Manager *manager)
{
    return &manager->entities[manager->selected_entity];
}

Entity *entity_iter_mouse_ray_collision(Entity_Manager *manager, Ray mouse_ray)
{
    RayCollision collision = {0};
    s32 best_dist = 1000000000;
    Entity *best = NULL;
    for (s32 i = 0; i < manager->current_len; i++) {
        Entity *e = &manager->entities[i];
        collision = entity_mouse_ray_collision(e, mouse_ray);
        if (collision.hit) {
            if (collision.distance < best_dist) best = e;
        }
    }

    return best;
}

s32 entity_iter_mouse_ray_collision_id(Entity_Manager *manager, Ray mouse_ray)
{
    RayCollision collision = {0};
    s32 best_dist = 1000000000;
    s32 best = 0;
    for (s32 i = 1; i < manager->current_len; i++) {
        Entity *e = &manager->entities[i];
        collision = entity_mouse_ray_collision(e, mouse_ray);
        if (collision.hit) {
            if (collision.distance < best_dist) best = i;
        }
    }

    return best;
}

void entity_manager_init(Entity_Manager *manager)
{
    // set one as start to skip 0 to have a null value;
    manager->current_len = 1;
}

void entity_manager_clear(Entity_Manager *manager)
{
    manager->current_len = 1;
}

static void update_shopkeeper(Entity *e)
{
	static f32 angle = 0;
    e->position.y = sin(angle);
	e->rotation = rotation_y(angle);
	angle += 0.01;
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

    if (e->update_fn) e->update_fn(e);
}

void entity_draw(Entity *e)
{
    if (e->anim_data)
        anim_apply(&e->anim, e->model);
    for (s32 i = 0; i < arrlen(e->model->primitives); i++) {
        draw_model(e->model->primitives[i], e->position, mat3_mul(e->rotation, mat3_scale(e->scale)), e->hit);
    }
}

RayCollision entity_mouse_ray_collision(Entity *e, Ray mouse_ray)
{
    RayCollision collision = {0};
    for (s32 i = 0; i < arrlen(e->model->primitives); i += 1) {
        Asset_Model *prim = e->model->primitives[i];
        for (s32 j = 0; j < arrlen(prim->vertices); j += 3) {
            Vertex v1 = prim->vertices[j];
            Vertex v2 = prim->vertices[j + 1];
            Vertex v3 = prim->vertices[j + 2];

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
    }

    return collision;
}

bool entity_serialize(FILE *f, const Entity *e) {
    bool ok = true;

    ok &= write_bytes(f, &e->tag,              sizeof(e->tag));

    ok &= write_string(f,  e->model_tag);   
    ok &= write_bytes(f, &e->position,         sizeof(e->position));
    ok &= write_bytes(f, &e->target,           sizeof(e->target));
    ok &= write_bytes(f, &e->rotation,         sizeof(e->rotation));
    ok &= write_bytes(f, &e->scale,            sizeof(e->scale));
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

    return ok;
}

bool entity_deserialize(Arena *arena, FILE *f, Entity *e) {
    bool ok = true;
    char buf[1024];

    ok &= read_bytes(f, &e->tag,             sizeof(e->tag));
    e->model = NULL;                                     
    ok &= read_string(f, buf, 1024);
    size_t len = strlen(buf) + 1;
    e->model_tag = push_array(arena, char, len);
    memcpy(e->model_tag, buf, len);
    e->model = get_model(e->model_tag);

    ok &= read_bytes(f, &e->position,        sizeof(e->position));
    ok &= read_bytes(f, &e->target,          sizeof(e->target));
    ok &= read_bytes(f, &e->rotation,        sizeof(e->rotation));
    ok &= read_bytes(f, &e->scale,           sizeof(e->scale));
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
    e->update_fn = NULL;                                

    return ok;
}
