#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "la.h"

#define EPSILON 1e30

typedef struct {
    V2f min, max;
} AABBf;

typedef struct {
    V2i min, max;
} AABBi;

typedef union {
    struct {f32 x, y, w, h;};
} Recf32;

typedef union {
    struct {s32 x, y, w, h;};
    struct {V2i min, max;};
} Recs32;


// todo:cs this needs to be visited for what happens when something is clipped behind the camera plane.
// right now it results in a divide by zero that will fail an assert.
static V2i          to_screen(V2f v);
static V2f          project(V3f v);
static inline AABBi calc_triangle_aabbi(V2i v1, V2i v2, V2i v3);
static bool         aabbi_collision(AABBi a, AABBi b);
static f32        signed_area(s32 ax, s32 ay, s32 bx, s32 by, s32 cx, s32 cy);
static bool         barycentric(V2i v1, V2i v2, V2i v3, V2i p, V3f *b);
static V2i          world_coord_to_2d(Camera c, V3f v);
static inline f32 deg_to_rad(f32 deg);

#endif // GEOMETRY_H
