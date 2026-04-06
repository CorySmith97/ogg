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

#include "la.h"
#include "assets.h"

typedef union Color {
    struct {
        u8 r, g, b, a;
    };
    u32 rgba;
} Color;


#define COLOR_WHITE  (Color){ 255, 255, 255, 255 }
#define COLOR_GRAY   (Color){ 122, 122, 122, 255 }
#define COLOR_BLACK  (Color){ 0, 0, 0, 255 }
#define COLOR_RED    (Color){.r = 252, .g = 44, .a = 255}
#define COLOR_BROWN  (Color){.r = 0x96, .g = 0x4B, .a = 0xFF }
#define COLOR_BLUE   (Color){.g = 127, .b = 252, .a = 255}
#define COLOR_GREEN  (Color){.r = 48, .g = 252, .a = 255}
#define COLOR_PURPLE (Color){.r = 0x50, .b = 0x50, .a = 255}
#define COLOR_YELLOW (Color){.r = 0x50, .g = 0x50, .a = 255}

typedef enum {
    TRIANGLE_TWO_D = 1 << 0,
    TRIANGLE_NO_CULLING = 1 << 1,
    TRIANGLE_WRITE_OVER_Z = 1 << 2,
} TriangleFlags;

typedef struct {
    V3f vertices[3];
    V3f normals[3];
    V3f uvs[3];
    Color colors[3];
    Texture *texture;
    u32 flags;
} Triangle;

typedef struct TextVertex {
    V2i pos;
    V2f uv;
} TextVertex;

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
    // TODO remove this from here
    Camera camera;
    Camera swap_camera; // spare camera to hold a different camera in
} renderer = {
    .quit = false,
    .width = GAME_WIDTH,
    .height = GAME_HEIGHT,
    .pixels = {0},
    .zbuffer = {0},
    .camera = {
        .target = {0, 0, 0},
        .position = {0,2,-2},
        .up = {0, 1, 0},
        .pitch = 45.0f,
        .yaw = 45.0f,
        .distance = 10.0f,
        .fovy = 60.0f,
    },
    .swap_camera = {
        .target = {0, 0, 0},
        .position = {0,2,-2},
        .up = {0, 1, 0},
        .pitch = 45.0f,
        .yaw = 45.0f,
        .distance = 10.0f,
        .fovy = 60.0f,
    },
};

void  render_init(void);
void  clear_background(Color color);
void  renderer_flush(void);
void  change_camera(void);

void  draw_model(Asset_Model *model, V3f position, Mat3 rotation);
void  draw_model_with_light(Asset_Model *model, V3f position, Mat3 rotation, Light light);
void  draw_model_textured(Asset_Model *model, V3f position, Mat3 rotation);
Color simple_reflection(SimpleMtl *mtl, V3f light_pos, V3f v, V3f n, V3f light_color, Color object_color);
void  draw_texture(Texture *tex, Recs32 rec);
void  draw_text(Font *f, const char *str, V2i pos, f32 size, Color color);
void  draw_rectangle3d(V3f bl, V3f br, V3f tl, V3f tr, Color color, u32 flags);
void  draw_triangle3d(V3f v1, V3f v2, V3f v3, Color color, u32 flags);
void  draw_texture3d(Texture *tex, V3f bl, V3f br, V3f tl, V3f tr, Color color, u32 flags);

#endif /* RENDER_H */
