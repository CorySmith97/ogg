#ifndef TILE_HEAD
#define TILE_HEAD

typedef enum {
    Tile_Dirt,
    Tile_Count,
} Tile_Tag;

typedef struct {
    V3i position;
    Color color;
    Texture *texture;
} Tile;

Tile tiles_types[Tile_Count];

void tiles_init(void);
void tile_draw(Tile *t);

#endif // TILE_H
