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

void entity_update(Entity *e)
{
    e->aabb.min = v3f(e->position.x - 0.25, e->position.y, e->position.z - 0.25);
    e->aabb.max = v3f(e->position.x + 0.25, e->position.y + 1, e->position.z + 0.25);
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
