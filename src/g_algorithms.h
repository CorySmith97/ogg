#ifndef ALGORITHMS_H
#define ALGORITHMS_H

typedef struct {

} Path;

typedef s32 (*hueristic)(s32 came_from, s32 current);

Path astar(Tile *tiles, V3f start, V3f end, hueristic h);

typedef enum {
    SORT_ASCENDING,
    SORT_DESCENDING,
} SortOrder;

s32 *sort_array_s32(s32 *array, SortOrder order);

#endif // ALGORITHMS_H
