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
        uint8_t r, g, b, a;
    };
    uint32_t rgba;
} Color;

#define COLOR_WHITE  (Color){ 255, 255, 255, 255 }
#define COLOR_BLACK  (Color){ 0, 0, 0, 255 }
#define COLOR_RED    (Color){.r = 0x50, .a = 255}
#define COLOR_BLUE   (Color){.b = 0x50, .a = 255}
#define COLOR_GREEN  (Color){.g = 0x50, .a = 255}
#define COLOR_PURPLE (Color){.r = 0x50, .b = 0x50, .a = 255}
#define COLOR_YELLOW (Color){.r = 0x50, .g = 0x50, .a = 255}

typedef struct {
    V3f vertices[3];
    V3f normals[3];
    V3f uvs[3];
    Color colors[3];
    Texture *texture;
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

typedef union {
    struct {int x, y, w, h;};
    struct {V2i min, max;};
} Reci;

static struct {
    Vertex *vertices;
    uint32_t pixels[GAME_HEIGHT * GAME_WIDTH];
    float zbuffer[GAME_HEIGHT * GAME_WIDTH];
    bool quit;
    int width, height;
    // TODO remove this from here
    Camera camera;
} renderer = {
    .quit = false,
    .width = GAME_WIDTH,
    .height = GAME_HEIGHT,
    .pixels = {0},
    .zbuffer = {0},
    .camera = {
        .position = {0,0,0},
        .up = {0, 1, 0},
        .front = {0, 0, 1},
        .pitch = 0.0f,
        .yaw = -90.0f,
    },
};

void  render_init(void);
void  clear_background(Color color);
void  draw_textured_triangle(Image *t, TextVertex v1, TextVertex v2, TextVertex v3);
void  renderer_draw_triangles(void);

void  draw_model(Asset_Model *model, V3f position, Mat3 rotation);
void  draw_model_with_light(Asset_Model *model, V3f position, Mat3 rotation, Light light);
void  draw_model_textured(Asset_Model *model, V3f position, Mat3 rotation);
Color simple_reflection(SimpleMtl *mtl, V3f light_pos, V3f v, V3f n, V3f light_color, Color object_color);
void  draw_texture(Texture *tex, Reci rec);
void  draw_text(Font *f, const char *str, V2i pos, float size, Color color);

#endif /* RENDER_H */
