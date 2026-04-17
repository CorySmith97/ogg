

void tiles_init(void)
{
    tiles_types[TILE_DIRT] = (Tile){
        .color = COLOR_BROWN,
        .texture = get_texture("dirt"),
    };
}

void tile_draw(Tile *t)
{
    V3f v1 = v3f(t->position.x - 0.5, t->position.y, t->position.z - 0.5);
    V3f v2 = v3f(t->position.x - 0.5, t->position.y, t->position.z + 0.5);
    V3f v3 = v3f(t->position.x + 0.5, t->position.y, t->position.z - 0.5);
    V3f v4 = v3f(t->position.x + 0.5, t->position.y, t->position.z + 0.5);

    draw_rectangle3d(v1, v2, v3, v4, COLOR_BROWN, 0);
}


RayCollision tile_mouse_ray_collision(Tile *t, Ray mouse_ray)
{
    RayCollision collision = {0};
    V3f v1 = v3f(t->position.x - 0.5, t->position.y, t->position.z - 0.5);
    V3f v2 = v3f(t->position.x - 0.5, t->position.y, t->position.z + 0.5);
    V3f v3 = v3f(t->position.x + 0.5, t->position.y, t->position.z - 0.5);
    V3f v4 = v3f(t->position.x + 0.5, t->position.y, t->position.z + 0.5);
    RayCollision c1 = get_ray_collision_triangle(mouse_ray, v3, v4, v2);
    RayCollision c2 = get_ray_collision_triangle(mouse_ray, v3, v2, v1);

    if (c1.hit) {
        collision = c1;
    }
    if (c2.hit) {
        collision = c2;
    }
    

    return collision;
}
