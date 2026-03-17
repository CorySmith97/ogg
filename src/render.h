#ifndef RENDER_H
#define RENDER_H

#include "la.h"
#include "assets.h"

#define WIDTH 640
#define HEIGHT 400

typedef struct Color 
{
    uint8_t r, g, b, a;
} Color;

#define COLOR_RED    (Color){.r = 255, .a = 255}
#define COLOR_BLUE   (Color){.b = 255, .a = 255}
#define COLOR_GREEN  (Color){.g = 255, .a = 255}
#define COLOR_PURPLE (Color){.r = 255, .b = 255, .a = 255}
#define COLOR_YELLOW (Color){.r = 255, .g = 255, .a = 255}

typedef struct TextVertex
{
    V2i pos;
    V2f uv;
} TextVertex;

static struct {
    Vertex *vertices;
    uint32_t pixels[HEIGHT][WIDTH];
    float zbuffer[HEIGHT][WIDTH];
    bool quit;
    int width, height;
} renderer = {
    .quit = false,
    .width = 640,
    .height = 400,
    .pixels = {0},
    .zbuffer = {0},
};

void     render_init(void);
void     clear_background(void);
void     set_pixel(uint32_t x, uint32_t y, Color color);
void     set_verline(uint32_t x, uint32_t start, uint32_t end, Color color);
void     set_line(V2i v, V2i u, Color color);
void     set_triangle(V2i v1, V2i v2, V2i v3, Color color);
void     set_triangle_multicolor(V2i v1, V2i v2, V2i v3, Color c1, Color c2, Color c3);
void     set_triangle_3d(V3f v1, V3f v2, V3f v3, Color color);
void     set_triangle_3d_multicolor(V3f v1, V3f v2, V3f v3, Color c1, Color c2, Color c3);
void     set_quad(V2i v1, V2i v2, V2i v3, V2i v4, Color c);
void     draw_textured_triangle(Image *t, TextVertex v1, TextVertex v2, TextVertex v3);
uint32_t pack_color(Color color);

#endif /* RENDER_H */
