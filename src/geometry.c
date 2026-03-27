#include "geometry.h"
#include "base.h"
#include <assert.h>

#define FOV 90.0f
#define FAR 5.0f
#define NEAR 0.1f
#define ASPECT_RATIO 16/9

static V2f project(V3f v)
{
    assert(v.z != 0);
    float fov_rad = 1.0f / tanf((FOV * 0.5f) * (M_PI / 180.0f));
    float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

    float x = (v.x * fov_rad) / (aspect * v.z);
    float y = (v.y * fov_rad) / v.z;
    return v2f(x, y);
    //return v2f(v.x / v.z, v.y / v.z);
}

static V2i to_screen(V2f v)
{
    float y = (1 - (v.y + 1) / 2) * GAME_HEIGHT;
    V2f ret = v2f((v.x + 1) / 2 * GAME_WIDTH, y);
    V2i val = v2i((int)ret.x, (int)ret.y);

    return val;
}

static inline AABBi calc_triangle_aabbi(V2i v1, V2i v2, V2i v3)
{
    AABBi ret = {0};
    ret.min.x = min(v1.x, min(v2.x, v3.x));
    ret.min.y = min(v1.y, min(v2.y, v3.y));

    ret.max.x = max(v1.x, max(v2.x, v3.x));
    ret.max.y = max(v1.y, max(v2.y, v3.y));

    return ret;
}

static bool aabbi_collision(AABBi a, AABBi b)
{
    return ((a.min.x <= b.max.x) || (a.min.y <= b.max.y) || (a.max.x >= b.min.x) || (a.max.y >= b.min.y));
}

static float signed_area(int ax, int ay,
                         int bx, int by,
                         int cx, int cy)
{
    return 0.5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}

static bool barycentric(V2i v1, V2i v2, V2i v3, V2i p, V3f *b)
{
    double sia = signed_area(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
    if (sia == 0)
        sia = -EPSILON;

    b->x = signed_area(p.x, p.y, v2.x, v2.y, v3.x, v3.y) / sia;
    b->y = signed_area(v1.x, v1.y, p.x, p.y, v3.x, v3.y) / sia;
    b->z = signed_area(v1.x, v1.y, v2.x, v2.y, p.x, p.y) / sia;
    if ((b->x >= 0 && b->x <= 1) && (b->y >= 0 && b->y <= 1) && (b->z >= 0 && b->z <= 1))
        return true;

    return false;
}

static inline float deg_to_rad(float deg)
{
    return deg * (M_PI/180.0f);
}
