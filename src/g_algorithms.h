#ifndef ALGORITHMS_H
#define ALGORITHMS_H

typedef struct {

} Path;

s32 (hueristic*)(s32 came_from, s32 current);

Path astar(Tile *tiles, V3f start, V3f end, hueristic h);

#endif // ALGORITHMS_H
