#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "la.h"

#define EPSILON 1e-30

typedef struct {
    V3f position;
    V3f direction;
} Ray;

typedef struct {
    bool hit;
    V3f normal;
    V3f point;
    f32 distance;
} RayCollision;

typedef struct {
    V2f min, max;
} AABBf;

typedef struct {
    V2i min, max;
} AABBi;

typedef struct {
    V3f min, max;
} AABB;

typedef union {
    struct {f32 x, y, w, h;};
} Recf;

typedef union {
    struct {s32 x, y, w, h;};
    struct {V2i min, max;};
} Recs32;

static V2i          to_screen(V2f v);
static V2f          project(V3f v);
static inline AABBi calc_triangle_aabbi(V2i v1, V2i v2, V2i v3);
static bool         aabbi_collision(AABBi a, AABBi b);
static f32        signed_area(s32 ax, s32 ay, s32 bx, s32 by, s32 cx, s32 cy);
static bool         barycentric(V2i v1, V2i v2, V2i v3, V2i p, V3f *b);
static V2i          world_coord_to_2d(Camera c, V3f v);
static inline f32 deg_to_rad(f32 deg);
static Ray        get_mouse_ray(Camera camera, V2f mouse_position);
static RayCollision get_raycollision_box(Ray ray, AABB box);
static RayCollision get_ray_collision_mesh(Ray ray, Asset_Model *box);
RayCollision get_ray_collision_triangle(Ray ray, V3f v1, V3f v2, V3f v3);
static inline AABB aabb_from_points(const V3f *pts, s32 count);
static inline AABB aabb_merge(AABB a, AABB b);
static inline AABB aabb_expand(AABB a, f32 e);

#endif // GEOMETRY_H
