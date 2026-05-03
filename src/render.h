/*
    Core Design aspects for the renderer as a whole. The renderer has a prepass to chunk triangles
    into buckets. The buckets are then dispatched to threads. Available threads query for available
    buckets that are not empty.

    Naming conventions:
        set_  is for internal drawing. Direct access to the canvas happens.

        draw_ is the public API for dispatching triangles into buckets.
 */
#ifndef RENDER_H
#define RENDER_H

typedef enum {
    BACKEND_SOFTWARE,
    BACKEND_OPENGL,
} RendererBackend;

typedef enum {
    TRIANGLE_TWO_D        = 1 << 0,
    TRIANGLE_NO_CULLING   = 1 << 1,
    TRIANGLE_WRITE_OVER_Z = 1 << 2,
    TRIANGLE_WIRE_FRAME   = 1 << 3,
    TRIANGLE_TEXT         = 1 << 4,
} TriangleFlags;

typedef struct {
    V3f vertices[3];
    V3f normals[3];
    V3f uvs[3];
    Color colors[3];
    Texture *texture;
	SimpleMtl *material;
    u32 flags;
} Triangle;

typedef struct {
    u32 len;
    u32 capacity;
    Triangle *data;
} TriangleArray;

typedef struct {
    u32 len;
    u32 capacity;
    size_t *data;
} SizeArray;


// Light color is a v3 for easier math with the materials as well as vector mathematics.
typedef struct {
    V3f position;
    V3f color;
} Light;

static struct {
    Vertex *vertices;
    u32 pixels[GAME_HEIGHT * GAME_WIDTH];
    f32 zbuffer[GAME_HEIGHT * GAME_WIDTH];
    b32 quit;
    s32 width, height;
    f32 dt;
    f64 time;

    // TODO remove this from here
    Camera *camera;
	Light sun;
    RendererBackend backend;
} renderer = {
    .quit = false,
    .width = GAME_WIDTH,
    .height = GAME_HEIGHT,
    .pixels = {0},
    .zbuffer = {0},
    .dt = 0,
    .camera = {
    },
    .backend = BACKEND_OPENGL,
};

//
// Render Initialization.
//
void render_init(void);
void render_shutdown(void);
void console_render_swap(String8 params);

//
// Clear all the triangle buffer and push pixels to render buffer
//
void renderer_flush(void);
void clear_background(Color color);
void change_camera(Camera *camera);

//
// Core rendering
// TODO there are some things here that could and should be reduced
//
void draw_model(Asset_Model *model, V3f position, Mat3 rotation, b32 selected);
void draw_model_with_light(Asset_Model *model, V3f position, Mat3 rotation, Light light);
void draw_texture(Texture *tex, Recs32 rec);
void draw_text(Font *f, const char *str, V2i pos, f32 size, Color color);
void draw_recs32(Recs32 rec, f32 z, Color color);
void draw_texture_w_uvs(Texture *tex, Recs32 rec, V3f uvs[4], Color colors[4], u32 flags);
void draw_rectangle3d(V3f bl, V3f br, V3f tl, V3f tr, Color color, u32 flags);
void draw_triangle3d(V3f v1, V3f v2, V3f v3, Color color, u32 flags);
void draw_multitriangle(V3f v1, V3f v2, V3f v3, Color c1, Color c2, Color c3, u32 flags);
void draw_multitriangle3d(V3f v1, V3f v2, V3f v3, Color c1, Color c2, Color c3, u32 flags);
void draw_texture3d(Texture *tex, V3f bl, V3f br, V3f tl, V3f tr, Color color, u32 flags);
void draw_string8(Font *f, String8 str, V2i pos, f32 size, Color color);
void draw_model_triangle_selection(Asset_Model *model, V3f position, Mat3 rotation, b32 *selected);

//
// Immediate Mode rendering
//


// TODO move to render global data
V3f   *immediate_vert = NULL;
Color *immediate_color = NULL;

void immediate_set_shader(String8 name);
void immediate_begin(void);
void immediate_flush(void);
void immediate_push_v(V3f v1, Color c);

#include "r_ui.h"

#endif /* RENDER_H */
