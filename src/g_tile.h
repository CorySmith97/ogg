#ifndef TILE_HEAD
#define TILE_HEAD

typedef enum {
    Tile_Dirt,
    Tile_Count,
} Tile_Tag;

typedef struct {
    V3f position;
    Color color;
    Texture *texture;
} Tile;

Tile tiles_types[Tile_Count];

void tiles_init(void);
void tile_draw(Tile *t);
RayCollision tile_mouse_ray_collision(Tile *t, Ray mouse_ray);

#endif // TILE_H
