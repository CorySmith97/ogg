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

typedef union Color
{
    struct
    {
        uint8_t r, g, b, a;
    };
    uint32_t rgba;
} Color;

#define COLOR_RED (Color){.r = 0x50, .a = 255}
#define COLOR_BLUE (Color){.b = 0x50, .a = 255}
#define COLOR_GREEN (Color){.g = 0x50, .a = 255}
#define COLOR_PURPLE (Color){.r = 0x50, .b = 0x50, .a = 255}
#define COLOR_YELLOW (Color){.r = 0x50, .g = 0x50, .a = 255}

typedef struct
{
    V3f vertices[3];
    Color colors[3];
    Image *image;
} Triangle;

typedef struct TextVertex
{
    V2i pos;
    V2f uv;
} TextVertex;

// Light color is a v3 for easier math with the materials as well as vector mathematics.
typedef struct
{
    V3f position;
    V3f color;
} Light;

static struct
{
    Vertex *vertices;
    uint32_t pixels[GAME_HEIGHT * GAME_WIDTH];
    float zbuffer[GAME_HEIGHT * GAME_WIDTH];
    bool quit;
    int width, height;
} renderer = {
    .quit = false,
    .width = GAME_WIDTH,
    .height = GAME_HEIGHT,
    .pixels = {0},
    .zbuffer = {0},
};

void render_init(void);
void clear_background(Color color);
static inline void set_pixel(int x, int y, Color color);
void set_verline(uint32_t x, uint32_t start, uint32_t end, Color color);
void set_line(V2i v, V2i u, Color color);
void set_triangle(V2i v1, V2i v2, V2i v3, Color color);
void set_triangle_multicolor(V2i v1, V2i v2, V2i v3, Color c1, Color c2, Color c3);
void set_triangle_3d(V3f v1, V3f v2, V3f v3, Color color);
void set_triangle_3d_multicolor(V3f v1, V3f v2, V3f v3, Color c1, Color c2, Color c3);
void set_quad(V2i v1, V2i v2, V2i v3, V2i v4, Color c);
void draw_textured_triangle(Image *t, TextVertex v1, TextVertex v2, TextVertex v3);
uint32_t pack_color(Color color);
void renderer_draw_triangle(uint32_t tile_x, uint32_t tile_y, Triangle tri);
void draw_model(Asset_Model *model, V3f position, Mat3 rotation);
void draw_model_with_light(Asset_Model *model, V3f position, Mat3 rotation, Light light);
Color simple_reflection(SimpleMtl *mtl, V3f light_pos, V3f v, V3f n, V3f light_color, Color object_color);

#endif /* RENDER_H */
