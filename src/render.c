#include "render.h"
#include <pthread.h>
#include <semaphore.h>

#define NUM_THREADS 12
#define TILE_W 64
#define TILE_H 64
#define COL_WIDTH (WIDTH / TILE_W)
#define TILE_COUNT ((WIDTH / TILE_W) * (HEIGHT / TILE_H))

typedef struct {
    int tile_x, tile_y;
    size_t *tri_idx;
} TileBin;

TileBin triangle_bins[(WIDTH / TILE_W) * (HEIGHT / TILE_H)];

Triangle *triangles = NULL;

// --- Thread pool ---
typedef struct {
    pthread_t thread;
    TileBin *bin;
    int active;
} Worker;

Worker workers[NUM_THREADS];
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  queue_cond  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  done_cond   = PTHREAD_COND_INITIALIZER;

TileBin *work_queue[TILE_COUNT];
int queue_head = 0;
int queue_tail = 0;
int jobs_remaining = 0;
int pool_running = 1;

void *worker_thread(void *arg)
{
    while (1) {
        pthread_mutex_lock(&queue_mutex);

        while (queue_head == queue_tail && pool_running)
            pthread_cond_wait(&queue_cond, &queue_mutex);

        if (!pool_running) {
            pthread_mutex_unlock(&queue_mutex);
            return NULL;
        }

        TileBin *bin = work_queue[queue_head % TILE_COUNT];
        queue_head++;
        pthread_mutex_unlock(&queue_mutex);

        size_t len = arrlen(bin->tri_idx);
        for (size_t i = 0; i < len; i++) {
            renderer_draw_triangle(
                bin->tile_x * TILE_W,
                bin->tile_y * TILE_H,
                triangles[bin->tri_idx[i]]
            );
        }
        arrsetlen(bin->tri_idx, 0);

        pthread_mutex_lock(&queue_mutex);
        jobs_remaining--;
        if (jobs_remaining == 0)
            pthread_cond_signal(&done_cond);
        pthread_mutex_unlock(&queue_mutex);
    }
}

void render_init(void)
{
    for (size_t i = 0; i < TILE_COUNT; i++) {
        int x = i % (WIDTH / TILE_W);
        int y = i / (WIDTH / TILE_W);
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
    for (int i = 0; i < TILE_COUNT; i++) {
        if (arrlen(triangle_bins[i].tri_idx) > 0) {
            work_queue[queue_tail % TILE_COUNT] = &triangle_bins[i];
            queue_tail++;
            jobs_remaining++;
        }
    }
    pthread_cond_broadcast(&queue_cond);  // wake all workers

    // wait for all tiles to finish
    while (jobs_remaining > 0)
        pthread_cond_wait(&done_cond, &queue_mutex);
    pthread_mutex_unlock(&queue_mutex);

    // clear triangle list for next frame
    arrsetlen(triangles, 0);
}

typedef struct {
    int tile_x, tile_y;
    TileBin *bin;
} ThreadArgs;

void thread_tile_work(void *args)
{
    ThreadArgs *targs = (ThreadArgs*)args;
    TileBin *bin = targs->bin;
    size_t len = arrlen(bin->tri_idx);
}

void thread_render_tile(void *args) 
{

}

uint32_t pack_color(Color color)
{
    return ((uint32_t)color.a << 24) |
           ((uint32_t)color.b << 16) |
           ((uint32_t)color.g << 8)  |
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

void set_pixel(uint32_t x, uint32_t y, Color color) 
{
    if (x >= (uint32_t)renderer.width 
            || x < 0 
            || y >= (uint32_t)renderer.height 
            || y < 0) return;

    renderer.pixels[x + y * WIDTH] = color.rgba;
}

void set_verline(uint32_t x,  uint32_t start, uint32_t end, Color color)
{
    for (uint32_t i = start; i < end; i++) {
        set_pixel(x, i, color);
    }
}

void set_line(V2i v, V2i u, Color color)
{
    bool steep = false;
    if (abs(v.x-v.y)<abs(u.x-u.y)) {
        V2i temp;
        temp = v;
        v = u;
        u = temp;
        steep = true;
    }

    int y = v.y;
    int ierror = 0;
    for (int x = v.x; x<u.x; x++) {
        if (steep)
            set_pixel(y, x, color);
        else 
            set_pixel(x, y, color);
        ierror += abs(u.y-v.y);
        if (ierror > u.x - v.x) {
            y += u.y > v.y ? 1 : -1;
            ierror -= 2 * (u.x-v.x);
        }
    }
}

void set_triangle(V2i v1, V2i v2, V2i v3, Color color)
{
    AABBi rec = {
        v2i(min(v1.x, min(v2.x, v3.x)), min(v1.y, min(v2.y, v3.y))),
        v2i(max(v1.x, max(v2.x, v3.x)), max(v1.y, max(v2.y, v3.y))),
    };
    V3f bary;

    for (uint32_t x = rec.min.x; x < rec.max.x; x++) {
        for (uint32_t y = rec.min.y; y < rec.max.y; y++) {
            if (barycentric(v1, v2, v3, v2i(x, y), &bary)) {
                set_pixel(x, y, color);
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

    for (int x = rec.min.x; x < rec.max.x; x++) {
        for (int y = rec.min.y; y < rec.max.y; y++) {
            if (barycentric(v1, v2, v3, v2i(x, y), &bary)) {
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
void renderer_push_triangle(V3f v1, V3f v2, V3f v3, Color color)
{
    V2i pos1 = to_screen(project(v1));
    V2i pos2 = to_screen(project(v2));
    V2i pos3 = to_screen(project(v3));

    AABBi rec = {
        v2i(min(pos1.x, min(pos2.x, pos3.x)), min(pos1.y, min(pos2.y, pos3.y))),
        v2i(max(pos1.x, max(pos2.x, pos3.x)), max(pos1.y, max(pos2.y, pos3.y))),
    };
    //logger(LOG_INFO, "Rec: %d %d %d %d", rec.min.x, rec.min.y, rec.max.x, rec.max.y);

    size_t bucket;

    size_t t_minx = floor(rec.min.x / TILE_W);
    size_t t_maxx = floor(rec.max.x / TILE_W);
    size_t t_miny = floor(rec.min.y / TILE_H);
    size_t t_maxy = floor(rec.max.y / TILE_H);

    Triangle triangle = {
        .vertices = {v1, v2, v3},
        .colors = {color},
        .image = NULL,
    };
    arrput(triangles, triangle);
    size_t idx = arrlen(triangles) - 1;

    for (size_t j = t_miny; j <= t_maxy; j++) {
        for (size_t i = t_minx; i <= t_maxx; i++) {
            arrput(triangle_bins[i + j * COL_WIDTH].tri_idx, idx);
        }
    }
}

void renderer_draw_triangle(uint32_t tile_x, uint32_t tile_y, Triangle tri) 
{
    V2i pos1 = to_screen(project(tri.vertices[0]));
    V2i pos2 = to_screen(project(tri.vertices[1]));
    V2i pos3 = to_screen(project(tri.vertices[2]));
    Color color = tri.colors[0];
    AABBi rec = {
        v2i(min(pos1.x, min(pos2.x, pos3.x)), min(pos1.y, min(pos2.y, pos3.y))),
        v2i(max(pos1.x, max(pos2.x, pos3.x)), max(pos1.y, max(pos2.y, pos3.y))),
    };

    V3f bary;

    double total_area = signed_area(pos1.x, pos1.y, pos2.x, pos2.y, pos3.x, pos3.y);
    if (total_area < -1e6) return; 


    for (int y = rec.min.y; y < rec.max.y; y++) {
        for (int x = rec.min.x; x < rec.max.x; x++) {
            if (barycentric(pos1, pos2, pos3, v2i(x, y), &bary)) {
                /* double z = (bary.x * tri.vertices[0].z + bary.y * tri.vertices[1].z + bary.z * tri.vertices[2].z);
                if (z <= renderer.zbuffer[x + y * WIDTH]) continue;
                renderer.zbuffer[x + y * WIDTH] = z;
                // normalize z to 0.0 - 1.0
                float t = (z - NEAR) / (FAR - NEAR);
                t = fmaxf(0.0f, fminf(1.0f, t)); // clamp

                unsigned char c = (unsigned char)(t * 255.0f); */
                
                if (x >= tile_x
                        && x < tile_x + TILE_W
                        && y >= tile_y
                        && y < tile_y + TILE_H)
                    set_pixel((uint32_t)x, (uint32_t)y, tri.colors[0]); //  (Color){c, c, c, 255});
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

    if (pos1.x < 0 || pos1.x > WIDTH 
        || pos2.x < 0 || pos2.x > WIDTH 
        || pos3.x < 0 || pos3.x > WIDTH 
        || pos1.y < 0 || pos1.y > HEIGHT 
        || pos2.y < 0 || pos2.y > HEIGHT 
        || pos3.y < 0 || pos3.y > HEIGHT) return;
    AABBi rec = {
        v2i(min(pos1.x, min(pos2.x, pos3.x)), min(pos1.y, min(pos2.y, pos3.y))),
        v2i(max(pos1.x, max(pos2.x, pos3.x)), max(pos1.y, max(pos2.y, pos3.y))),
    };
    V3f bary;

    double total_area = signed_area(pos1.x, pos1.y, pos2.x, pos2.y, pos3.x, pos3.y);
    if (total_area < -1e6) return; 


    for (int y = rec.min.y; y < rec.max.y; y++) {
        for (int x = rec.min.x; x < rec.max.x; x++) {
            if (barycentric(pos1, pos2, pos3, v2i(x, y), &bary)) {
                double z = (bary.x * v1.z + bary.y * v2.z + bary.z * v3.z);
                //logger(LOG_DEBUG, "z: %c, zbuf: %f", z, renderer.zbuffer[y][x]);
                if (z <= renderer.zbuffer[x + y * WIDTH]) continue;
                renderer.zbuffer[x + y * WIDTH] = z;
                // normalize z to 0.0 - 1.0
                float t = (z - NEAR) / (FAR - NEAR);
                t = fmaxf(0.0f, fminf(1.0f, t)); // clamp

                unsigned char c = (unsigned char)(t * 255.0f);
                
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

void clear_background(void)
{
    TileBin bin;
    for (size_t i = 0; i < sizeof(triangle_bins)/sizeof(triangle_bins[0]); i++) {
        bin = triangle_bins[i];
    }

    memset(renderer.pixels, 0, WIDTH * HEIGHT * sizeof(uint32_t));

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            renderer.zbuffer[x + y * WIDTH] = -1e10;
        }
    }
}

