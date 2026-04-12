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

void entity_update(Entity *e)
{
    e->position = v3f_add(e->position, v3f_scale(v3f_sub(e->target, e->position), 0.1));
    // TODO is the angle is too wide then it doesnt rotate properly
    V3f dir = v3f_sub(e->target, e->position);

    if (!v3f_equal(dir, v3f(0,0,0))) {
        dir = v3f_normalize(dir);

        // assuming forward is +Z
        f32 yaw = atan2f(dir.x, -dir.z);

        e->rotation = rotation_y(yaw);
    }
    e->update_fn(e);
}

void entity_draw(Entity *e)
{
    draw_model(e->model, e->position, e->rotation);
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
