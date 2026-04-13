#ifndef TILE_HEAD
#define TILE_HEAD

typedef enum {
    TILE_DIRT,
    TILE_COUNT,
} Tile_Tag;

typedef struct {
    Recs32 rec;
    V3f position;
    Color color;
    Texture *texture;
} Tile;

Tile tiles_types[TILE_COUNT];

void tiles_init(void);
void tile_draw(Tile *t);
RayCollision tile_mouse_ray_collision(Tile *t, Ray mouse_ray);
b32 tile_serialize(FILE *f, const Tile *t);
b32 tile_deserialize(Arena *arena, Tile *t);

#endif // TILE_H
