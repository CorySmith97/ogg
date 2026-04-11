

void tiles_init(void)
{
    tiles_types[Tile_Dirt] = (Tile){
        .color = COLOR_BROWN,
        .texture = NULL,
    };
}

void tile_draw(Tile *t)
{
}


RayCollision tile_mouse_ray_collision(Tile *t, Ray mouse_ray)
{
    RayCollision collision = {0};
    

    return collision;
}
