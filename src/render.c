#include "render.h"
#include <pthread.h>
#include <semaphore.h>
#include <float.h>

#define NUM_THREADS 12
#define TILE_W 64
#define TILE_H 64
#define COL_GAME_WIDTH (GAME_WIDTH / TILE_W)
#define TILE_COUNT ((GAME_WIDTH / TILE_W) * (GAME_HEIGHT / TILE_H))

typedef struct
{
    int tile_x, tile_y;
    size_t *tri_idx;
} TileBin;

// --- Thread pool ---
typedef struct
{
    pthread_t thread;
    TileBin *bin;
    int active;
} Worker;

Color phong_reflection(V3f light, V3f n1, V3f n2, V3f n3);

TileBin triangle_bins[(GAME_WIDTH / TILE_W) * (GAME_HEIGHT / TILE_H)];
Triangle *triangles = NULL;
Worker workers[NUM_THREADS];
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t done_cond = PTHREAD_COND_INITIALIZER;

TileBin *work_queue[TILE_COUNT];
int queue_head = 0;
int queue_tail = 0;
int jobs_remaining = 0;
int pool_running = 1;

void *worker_thread(void *arg)
{
    UNUSED(arg);
    while (1)
    {
        pthread_mutex_lock(&queue_mutex);

        while (queue_head == queue_tail && pool_running)
            pthread_cond_wait(&queue_cond, &queue_mutex);

        if (!pool_running && queue_head == queue_tail)
        {
            pthread_mutex_unlock(&queue_mutex);
            return NULL;
        }

        // Grab bin pointer AND increment while still holding the lock
        TileBin *bin = work_queue[queue_head % TILE_COUNT];
        queue_head++;

        // Snapshot the triangle indices we need to process
        // so we can release the lock before doing pixel work
        size_t len = arrlen(bin->tri_idx);

        // TODO this needs to be a fixed buffer of some kind.
        size_t local_idx[len]; // VLA, or heap alloc if len could be huge
        memcpy(local_idx, bin->tri_idx, len * sizeof(size_t));
        arrsetlen(bin->tri_idx, 0); // clear while locked, before workers
                                    // can re-queue this bin next frame

        pthread_mutex_unlock(&queue_mutex);

        for (size_t i = 0; i < len; i++)
        {
            renderer_draw_triangle(
                bin->tile_x * TILE_W,
                bin->tile_y * TILE_H,
                triangles[local_idx[i]] // triangles[] is read-only during dispatch
            );
        }

        pthread_mutex_lock(&queue_mutex);
        jobs_remaining--;
        if (jobs_remaining == 0)
            pthread_cond_signal(&done_cond);
        pthread_mutex_unlock(&queue_mutex);
    }
}

void render_init(void)
{
    renderer.width = GAME_WIDTH;
    renderer.height = GAME_HEIGHT;
    for (size_t i = 0; i < TILE_COUNT; i++)
    {
        int x = i % (GAME_WIDTH / TILE_W);
        int y = i / (GAME_WIDTH / TILE_W);
        triangle_bins[i].tile_x = x;
        triangle_bins[i].tile_y = y;
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&workers[i].thread, NULL, worker_thread, NULL);
}

void render_shutdown(void)
{
    pthread_mutex_lock(&queue_mutex);
    pool_running = 0;
    pthread_cond_broadcast(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(workers[i].thread, NULL);
}

void renderer_draw_triangles(void)
{
    // enqueue all non-empty bins
    pthread_mutex_lock(&queue_mutex);
    for (int i = 0; i < TILE_COUNT; i++)
    {
        if (arrlen(triangle_bins[i].tri_idx) > 0)
        {
            work_queue[queue_tail % TILE_COUNT] = &triangle_bins[i];
            queue_tail++;
            jobs_remaining++;
        }
    }
    pthread_cond_broadcast(&queue_cond); // wake all workers

    // wait for all tiles to finish
    while (jobs_remaining > 0)
        pthread_cond_wait(&done_cond, &queue_mutex);
    pthread_mutex_unlock(&queue_mutex);

    // clear triangle list for next frame
    arrsetlen(triangles, 0);
}

uint32_t pack_color(Color color)
{
    return ((uint32_t)color.a << 24) |
           ((uint32_t)color.b << 16) |
           ((uint32_t)color.g << 8) |
           ((uint32_t)color.r);
}

Color color_scale(Color c, double value)
{
    return (Color){
        c.r * value,
        c.g * value,
        c.b * value,
        c.a,
    };
}

Color color_add(Color c1, Color c2)
{
    return (Color){
        c1.r + c2.r,
        c1.g + c2.g,
        c1.b + c2.b,
        c1.a + c2.a,
    };
}

Color color_mul(Color c1, Color c2)
{
    return (Color){
        c1.r * c2.r,
        c1.g * c2.g,
        c1.b * c2.b,
        c1.a * c2.a,
    };
}

#define PIXEL_INDEX(x, y) ((size_t)(x) + (size_t)(y) * (size_t)renderer.width)

static inline void set_pixel(int x, int y, Color color)
{
    if (x < 0 || y < 0 || x >= renderer.width || y >= renderer.height)
        return;

    renderer.pixels[PIXEL_INDEX(x, y)] = color.rgba;
}

void set_verline(uint32_t x, uint32_t start, uint32_t end, Color color)
{
    for (uint32_t i = start; i < end; i++)
    {
        set_pixel(x, i, color);
    }
}

void set_line(V2i v, V2i u, Color color)
{
    bool steep = false;
    if (abs(v.x - v.y) < abs(u.x - u.y))
    {
        V2i temp;
        temp = v;
        v = u;
        u = temp;
        steep = true;
    }

    int y = v.y;
    int ierror = 0;
    for (int x = v.x; x < u.x; x++)
    {
        if (steep)
            set_pixel(y, x, color);
        else
            set_pixel(x, y, color);
        ierror += abs(u.y - v.y);
        if (ierror > u.x - v.x)
        {
            y += u.y > v.y ? 1 : -1;
            ierror -= 2 * (u.x - v.x);
        }
    }
}

bool check_bounds(V2i v1, V2i v2, V2i v3)
{
    if (v1.x < 0 || v1.x > GAME_WIDTH || v2.x < 0 || v2.x > GAME_WIDTH || v3.x < 0 || v3.x > GAME_WIDTH || v1.y < 0 || v1.y > GAME_HEIGHT || v2.y < 0 || v2.y > GAME_HEIGHT || v3.y < 0 || v3.y > GAME_HEIGHT)
        return false;

    return true;
}

void set_triangle(V2i v1, V2i v2, V2i v3, Color color)
{
    AABBi rec = {
        v2i(min(v1.x, min(v2.x, v3.x)), min(v1.y, min(v2.y, v3.y))),
        v2i(max(v1.x, max(v2.x, v3.x)), max(v1.y, max(v2.y, v3.y))),
    };
    V3f bary;

    for (int x = rec.min.x; x < rec.max.x; x++)
    {
        for (int y = rec.min.y; y < rec.max.y; y++)
        {
            if (barycentric(v1, v2, v3, v2i(x, y), &bary))
            {
                set_pixel((uint32_t)x, (uint32_t)y, color);
            }
        }
    }
}

void set_triangle_multicolor(V2i v1, V2i v2, V2i v3, Color c1, Color c2, Color c3)
{
    AABBi rec = {
        v2i(min(v1.x, min(v2.x, v3.x)), min(v1.y, min(v2.y, v3.y))),
        v2i(max(v1.x, max(v2.x, v3.x)), max(v1.y, max(v2.y, v3.y))),
    };
    V3f bary;

    for (int x = rec.min.x; x < rec.max.x; x++)
    {
        for (int y = rec.min.y; y < rec.max.y; y++)
        {
            if (barycentric(v1, v2, v3, v2i(x, y), &bary))
            {
                Color color = color_add(color_add(color_scale(c1, bary.x), color_scale(c2, bary.y)), color_scale(c3, bary.z));
                set_pixel((uint32_t)x, (uint32_t)y, color);
            }
        }
    }
}
/*
void renderer_draw_triangles(void)
{
    int tri_count = 0;
    Triangle tri;
    TileBin bin;
    for (size_t i = 0; i < sizeof(triangle_bins)/sizeof(triangle_bins[0]); i++) {
         bin = triangle_bins[i];
         size_t len = arrlen(bin.tri_idx);
         tri_count += len;
         //if (len > 0) logger(LOG_INFO, "BIN LENGTH: %zu", len);
         for (size_t idx = 0; idx < len; idx++) {
            size_t inx = arrpop(bin.tri_idx);
            renderer_draw_triangle(bin.tile_x * TILE_W, bin.tile_y * TILE_H, triangles[inx]);
         }
    }
    logger(LOG_INFO, "total triangles renderered: %d", tri_count);
}
 */
void renderer_push_triangle(V3f v1, V3f v2, V3f v3, Color color[3])
{
    V2i pos1 = to_screen(project(v1));
    V2i pos2 = to_screen(project(v2));
    V2i pos3 = to_screen(project(v3));
    if (!check_bounds(pos1, pos2, pos3))
        return;

    AABBi rec = {
        v2i(min(pos1.x, min(pos2.x, pos3.x)), min(pos1.y, min(pos2.y, pos3.y))),
        v2i(max(pos1.x, max(pos2.x, pos3.x)), max(pos1.y, max(pos2.y, pos3.y))),
    };

    size_t t_minx = max(0, floor(rec.min.x / TILE_W));
    size_t t_maxx = min((GAME_WIDTH / TILE_W) - 1, floor(rec.max.x / TILE_W));
    size_t t_miny = max(0, floor(rec.min.y / TILE_H));
    size_t t_maxy = min((GAME_HEIGHT / TILE_H) - 1, floor(rec.max.y / TILE_H));

    if (t_minx > t_maxx || t_miny > t_maxy)
        return;

    Triangle triangle = {
        .vertices = {v1, v2, v3},
        .colors = {color[0], color[1], color[2]},
        .image = NULL,
    };
    arrput(triangles, triangle);
    size_t idx = arrlen(triangles) - 1;

    for (size_t j = t_miny; j <= t_maxy; j++)
    {
        for (size_t i = t_minx; i <= t_maxx; i++)
        {
            arrput(triangle_bins[i + j * COL_GAME_WIDTH].tri_idx, idx);
        }
    }
}


void renderer_push_triangle_w_normals(V3f positions[3], V3f normals[3], Color color[3])
{
    V2i pos1 = to_screen(project(positions[0]));
    V2i pos2 = to_screen(project(positions[1]));
    V2i pos3 = to_screen(project(positions[2]));
    if (!check_bounds(pos1, pos2, pos3))
        return;

    AABBi rec = {
        v2i(min(pos1.x, min(pos2.x, pos3.x)), min(pos1.y, min(pos2.y, pos3.y))),
        v2i(max(pos1.x, max(pos2.x, pos3.x)), max(pos1.y, max(pos2.y, pos3.y))),
    };

    size_t t_minx = max(0, floor(rec.min.x / TILE_W));
    size_t t_maxx = min((GAME_WIDTH / TILE_W) - 1, floor(rec.max.x / TILE_W));
    size_t t_miny = max(0, floor(rec.min.y / TILE_H));
    size_t t_maxy = min((GAME_HEIGHT / TILE_H) - 1, floor(rec.max.y / TILE_H));

    if (t_minx > t_maxx || t_miny > t_maxy)
        return;

    Triangle triangle = {
        .vertices = {positions[0], positions[1], positions[2]},
        .normals = {normals[0], normals[1], normals[2]},
        .colors = {color[0], color[1], color[2]},
        .image = NULL,
    };
    arrput(triangles, triangle);
    size_t idx = arrlen(triangles) - 1;

    for (size_t j = t_miny; j <= t_maxy; j++)
    {
        for (size_t i = t_minx; i <= t_maxx; i++)
        {
            arrput(triangle_bins[i + j * COL_GAME_WIDTH].tri_idx, idx);
        }
    }
}

void renderer_draw_triangle(uint32_t tile_x, uint32_t tile_y, Triangle tri)
{
    V2i pos1 = to_screen(project(tri.vertices[0]));
    V2i pos2 = to_screen(project(tri.vertices[1]));
    V2i pos3 = to_screen(project(tri.vertices[2]));

    AABBi rec = {
        v2i(min(pos1.x, min(pos2.x, pos3.x)), min(pos1.y, min(pos2.y, pos3.y))),
        v2i(max(pos1.x, max(pos2.x, pos3.x)), max(pos1.y, max(pos2.y, pos3.y))),
    };

    int x0 = max(rec.min.x, (int)tile_x);
    int x1 = min(rec.max.x, (int)(tile_x + TILE_W));
    int y0 = max(rec.min.y, (int)tile_y);
    int y1 = min(rec.max.y, (int)(tile_y + TILE_H));

    V3f bary;

    double total_area = signed_area(pos1.x, pos1.y, pos2.x, pos2.y, pos3.x, pos3.y);
    if (total_area < -1e6)
        return;

    for (int y = y0; y < y1; y++)
    {
        for (int x = x0; x < x1; x++)
        {
            if (barycentric(pos1, pos2, pos3, v2i(x, y), &bary))
            {
                double inv_z = (bary.x / tri.vertices[0].z +
                                bary.y / tri.vertices[1].z +
                                bary.z / tri.vertices[2].z);
                double z = 1.0 / inv_z;

                size_t idx = PIXEL_INDEX(x, y);

                if (z >= renderer.zbuffer[idx])
                    continue;
                renderer.zbuffer[idx] = z;

                // TODO Interpolate colors

                Color int_color = color_add(
                    color_add(
                        color_scale(tri.colors[0], bary.x),
                        color_scale(tri.colors[1], bary.y)),
                    color_scale(tri.colors[2], bary.z));
                set_pixel((uint32_t)x, (uint32_t)y, int_color); //  (Color){c, c, c, 255});
            }
        }
    }
}

void draw_point(V3f p, Color color)
{
    V2i pos = to_screen(project(p));

    set_pixel(pos.x, pos.y, color);
}

void set_triangle_3d(V3f v1, V3f v2, V3f v3, Color color)
{
    V2i pos1 = to_screen(project(v1));
    V2i pos2 = to_screen(project(v2));
    V2i pos3 = to_screen(project(v3));

    AABBi rec = {
        v2i(min(pos1.x, min(pos2.x, pos3.x)), min(pos1.y, min(pos2.y, pos3.y))),
        v2i(max(pos1.x, max(pos2.x, pos3.x)), max(pos1.y, max(pos2.y, pos3.y))),
    };
    V3f bary;

    double total_area = signed_area(pos1.x, pos1.y, pos2.x, pos2.y, pos3.x, pos3.y);
    if (total_area < -1e6)
        return;

    for (int y = rec.min.y; y < rec.max.y; y++)
    {
        for (int x = rec.min.x; x < rec.max.x; x++)
        {
            if (barycentric(pos1, pos2, pos3, v2i(x, y), &bary))
            {
                double z = (bary.x * v1.z + bary.y * v2.z + bary.z * v3.z);
                // logger(LOG_DEBUG, "z: %c, zbuf: %f", z, renderer.zbuffer[y][x]);
                if (z < renderer.zbuffer[x + y * GAME_WIDTH])
                    continue;
                renderer.zbuffer[x + y * GAME_WIDTH] = z;

                set_pixel((uint32_t)x, (uint32_t)y, color); // (Color){c, c, c, 255});
            }
        }
    }
}

// Top right, bottom right, top left, bottom left
void set_quad(V2i v1, V2i v2, V2i v3, V2i v4, Color c)
{
    set_triangle(v1, v2, v3, c);
    set_triangle(v1, v3, v4, c);
}

Color get_color_from_image(Image *i, V2f uv)
{
    Color c = {0};
    size_t x = uv.x * i->width;
    size_t y = uv.y * i->height;

    c.r = i->pixels[x + y];
    c.g = i->pixels[x + y + 1];
    c.b = i->pixels[x + y + 2];
    c.a = i->pixels[x + y + 3];

    return c;
}

/* void draw_textured_triangle(Image *t, Vertex v1, Vertex v2, Vertex v3)
{
    UNUSED(t);

    AABBi rec = {
        v2i(min(v1.pos.x, min(v2.pos.x, v3.pos.x)), min(v1.pos.y, min(v2.pos.y, v3.pos.y))),
        v2i(max(v1.pos.x, max(v2.pos.x, v3.pos.x)), max(v1.pos.y, max(v2.pos.y, v3.pos.y))),
    };
    V3f bary;

    for (int x = rec.min.x; x < rec.max.x; x++) {
        for (int y = rec.min.y; y < rec.max.y; y++) {
            if (barycentric(v1.pos, v2.pos, v3.pos, v2i(x, y), &bary)) {
                V2f uv1 = v2f_scale(v1.uv, bary.x);
                V2f uv2 = v2f_scale(v2.uv, bary.x);
                V2f uv3 = v2f_scale(v3.uv, bary.x);
                UNUSED(uv1);
                UNUSED(uv2);
                UNUSED(uv3);
                // @todo:cs get color from texture.
                //Color color = color_add(color_add(, color_scale(c2, bary.y)), color_scale(c3, bary.z));

                //set_pixel((uint32_t)x, (uint32_t)y, color);
            }
        }
    }

} */

void clear_background(Color color)
{
    for (int y = 0; y < renderer.height; y++)
    {
        for (int x = 0; x < renderer.width; x++)
        {
            size_t idx = PIXEL_INDEX(x, y);
            renderer.pixels[idx] = color.rgba;
            renderer.zbuffer[idx] = FLT_MAX;
        }
    }
}

void draw_model(Asset_Model *model, V3f position, Mat3 rotation)
{
    V3f light_pos = (V3f){1, 0, 2};

    for (int i = 0; i < arrlen(model->vertices); i += 3)
    {
        Vertex v1 = model->vertices[i];
        Vertex v2 = model->vertices[i + 1];
        Vertex v3 = model->vertices[i + 2];

        V3f p1 = v3f_mul_mat3(v1.position, rotation);
        V3f p2 = v3f_mul_mat3(v2.position, rotation);
        V3f p3 = v3f_mul_mat3(v3.position, rotation);

        p1.z = -p1.z;
        p2.z = -p2.z;
        p3.z = -p3.z;
        p1 = v3f_add(p1, position);
        p2 = v3f_add(p2, position);
        p3 = v3f_add(p3, position);

        if (p1.z <= NEAR || p2.z <= NEAR || p3.z <= NEAR)
            continue;
        light_pos.z += 0.01;
        Color color;
        color.r = (uint8_t)((v1.normal.x * 0.5f + 0.5f) * 255);
        color.g = (uint8_t)((v1.normal.y * 0.5f + 0.5f) * 255);
        color.b = (uint8_t)((v1.normal.z * 0.5f + 0.5f) * 255);
        color.a = 255;
        Color colors[3] = {color};

        renderer_push_triangle(
            p1,
            p2,
            p3,
            colors);
    }
}

void draw_model_with_light(Asset_Model *model, V3f position, Mat3 rotation, Light light)
{
    for (int i = 0; i < arrlen(model->vertices); i += 3)
    {
        Vertex v1 = model->vertices[i];
        Vertex v2 = model->vertices[i + 1];
        Vertex v3 = model->vertices[i + 2];

        V3f p1 = v3f_mul_mat3(v1.position, rotation);
        V3f p2 = v3f_mul_mat3(v2.position, rotation);
        V3f p3 = v3f_mul_mat3(v3.position, rotation);

        V3f n1 = v3f_mul_mat3(v1.normal, rotation);
        V3f n2 = v3f_mul_mat3(v2.normal, rotation);
        V3f n3 = v3f_mul_mat3(v3.normal, rotation);

        p1.z = -p1.z;
        p2.z = -p2.z;
        p3.z = -p3.z;
        p1 = v3f_add(p1, position);
        p2 = v3f_add(p2, position);
        p3 = v3f_add(p3, position);

        if (p1.z <= NEAR || p2.z <= NEAR || p3.z <= NEAR)
            continue;

        Color c1 = simple_reflection(model->mtl, light.position, p1, n1, light.color, COLOR_GREEN);
        Color c2 = simple_reflection(model->mtl, light.position, p2, n2, light.color, COLOR_GREEN);
        Color c3 = simple_reflection(model->mtl, light.position, p3, n3, light.color, COLOR_GREEN);

        Color colors[3] = {c1, c2, c3};
        renderer_push_triangle(
            p1,
            p2,
            p3,
            colors);
    }
}

Color simple_reflection(SimpleMtl *mtl, V3f light_pos, V3f v, V3f n, V3f light_color, Color object_color)
{
    if (mtl == NULL)
        return object_color;
    V3f norm = v3f_normalize(n);
    V3f light_dir = v3f_normalize(v3f_sub(light_pos, v));

    float diff = fmaxf(v3f_dot(norm, light_dir), 0.0f);

    V3f obj = {
        object_color.r / 255.0,
        object_color.g / 255.0,
        object_color.b / 255.0,
    };
    V3f diffuse = v3f_scale(light_color, diff);
    V3f result = v3f_mul(v3f_add(diffuse, mtl->ambient), obj);

    // clamp to [0, 1] before converting to avoid overflow
    Color res = (Color){
        .r = (uint8_t)(fminf(result.x, 1.0f) * 255),
        .g = (uint8_t)(fminf(result.y, 1.0f) * 255),
        .b = (uint8_t)(fminf(result.z, 1.0f) * 255),
        .a = 255,
    };
    return res;
}
