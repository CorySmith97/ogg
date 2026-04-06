#include "geometry.h"
#include "base.h"
#include <assert.h>

#define FOV 90.0f
#define FAR 5.0f
#define NEAR 0.1f
#define ASPECT_RATIO 16/9

static V2f project(V3f v)
{
    float z = (v.z < NEAR) ? NEAR : v.z;
    assert(v.z != 0);
    float fov_rad = 1.0f; //1.0f / tanf((renderer.camera.fovy * 0.5f) * (M_PI / 180.0f));
    float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

    float x = (v.x * fov_rad) / (aspect * z);
    float y = (v.y * fov_rad) / z;
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


static Ray get_mouse_ray(Camera camera, V2f mouse_position)
{
    Ray ray = { 0 };

    // Calculate normalized device coordinates
    // NOTE: y value is negative
    float x = (2.0f*mouse_position.x)/(float)SCREEN_WIDTH - 1.0f;
    float y = 1.0f - (2.0f*mouse_position.y)/(float)SCREEN_HEIGHT;
    float z = 1.0f;

    // Store values in a vector
    V3f deviceCoords = { x, y, z };

    // Calculate view matrix from camera look at
    Mat4 matView = look_at(camera.position, camera.target, camera.up);

    Mat4 matProj = mat4_identity();

    // Calculate projection matrix from perspective
    matProj = perspective(NEAR, FAR,  ((double)SCREEN_WIDTH/(double)SCREEN_HEIGHT), camera.fovy);

    // Unproject far/near points
    V3f nearPoint = v3f_unproject(v3f( deviceCoords.x, deviceCoords.y, 0.0f ), matProj, matView);
    V3f farPoint = v3f_unproject(v3f( deviceCoords.x, deviceCoords.y, 1.0f ), matProj, matView);

    V3f direction = v3f_normalize(v3f_sub(farPoint, nearPoint));

    // Reverse direction - ray is pointing backwards
    direction = v3f_scale(direction, -1.0f);

     ray.position = camera.position;

    // Apply calculated vectors to ray
    ray.direction = direction;

    return ray;
}

// TAKEN FROM RAYLIB

RayCollision get_raycollision_box(Ray ray, AABB box)
{
    RayCollision collision = { 0 };

    // NOTE: If ray.position is inside the box, the distance is negative (as if the ray was reversed)
    // Reversing ray.direction will give use the correct result
    bool insideBox = (ray.position.x > box.min.x) && (ray.position.x < box.max.x) &&
                     (ray.position.y > box.min.y) && (ray.position.y < box.max.y) &&
                     (ray.position.z > box.min.z) && (ray.position.z < box.max.z);

    if (insideBox) ray.direction = v3f_scale(ray.direction, -1);

    float t[11] = { 0 };

    float epsilon = 1e-6f;
    t[8] = (fabsf(ray.direction.x) < epsilon) ? 1e30f : 1.0f/ray.direction.x;
    t[9] = (fabsf(ray.direction.y) < epsilon) ? 1e30f : 1.0f/ray.direction.y;
    t[10] = (fabsf(ray.direction.z) < epsilon) ? 1e30f : 1.0f/ray.direction.z;

    t[0] = (box.min.x - ray.position.x)*t[8];
    t[1] = (box.max.x - ray.position.x)*t[8];
    t[2] = (box.min.y - ray.position.y)*t[9];
    t[3] = (box.max.y - ray.position.y)*t[9];
    t[4] = (box.min.z - ray.position.z)*t[10];
    t[5] = (box.max.z - ray.position.z)*t[10];
    t[6] = (float)fmax(fmax(fmin(t[0], t[1]), fmin(t[2], t[3])), fmin(t[4], t[5]));
    t[7] = (float)fmin(fmin(fmax(t[0], t[1]), fmax(t[2], t[3])), fmax(t[4], t[5]));


    collision.hit = (t[7] >= 0) && (t[6] <= t[7]);
    collision.distance = t[6];
    collision.point = v3f_add(ray.position, v3f_scale(ray.direction, collision.distance));

    if (collision.hit) {
    char debug_buf[256];
    sprintf(debug_buf, "RAYCOL: Box(%.1f,%.1f,%.1f)-(%.1f,%.1f,%.1f)", box.min.x, box.min.y, box.min.z, box.max.x, box.max.y, box.max.z);
    log_debug(debug_buf);
    sprintf(debug_buf, "RAYCOL: Ray pos(%.1f,%.1f,%.1f) dir(%.1f,%.1f,%.1f)", ray.position.x, ray.position.y, ray.position.z, ray.direction.x, ray.direction.y, ray.direction.z);
    log_debug(debug_buf);
    sprintf(debug_buf, "RAYCOL: t[6]=%.1f t[7]=%.1f hit=%d", t[6], t[7], collision.hit);
    log_debug(debug_buf);
    }


    /* // Get box center point
    collision.normal = Vector3Lerp(box.min, box.max, 0.5f);
    // Get vector center point->hit point
    collision.normal = Vector3Subtract(collision.point, collision.normal);
    // Scale vector to unit cube
    // NOTE: Use an additional .01 to fix numerical errors
    collision.normal = Vector3Scale(collision.normal, 2.01f);
    collision.normal = Vector3Divide(collision.normal, Vector3Subtract(box.max, box.min));
    // The relevant elements of the vector are now slightly larger than 1.0f (or smaller than -1.0f)
    // and the others are somewhere between -1.0 and 1.0 casting to int is exactly our wanted normal!
    collision.normal.x = (float)((int)collision.normal.x);
    collision.normal.y = (float)((int)collision.normal.y);
    collision.normal.z = (float)((int)collision.normal.z);

    collision.normal = Vector3Normalize(collision.normal); */

    if (insideBox)
    {
        // Reset ray.direction
        ray.direction = v3f_scale(ray.direction, -1);
        // Fix result
        collision.distance *= -1.0f;
        //collision.normal = v3f_scale(collision.normal, -1.0f);
    }

    return collision;
}

