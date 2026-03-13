#ifndef RENDER_H
#define RENDER_H

#include "la.h"

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

typedef struct Vertex
{
    V3f position;
    V3f normal;
    V2f uv;
} Vertex;

// apply camera transform
// need 3d to 2d
// 2d to screen
//
//
// @todo:cs this needs to be moved to a platform layer
static struct {
    SDL_Window *window;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
    Vertex *vertices;
    uint32_t pixels[HEIGHT][WIDTH];
    bool quit;
    int width, height;
} renderer = {
    .quit = false,
    .width = 640,
    .height = 400,
    .pixels = {0},
};

void render_init(void);
void clear_background(void);
// @todo:cs this needs to be moved to a platform layer
void present(void);
void set_pixel(uint32_t x, uint32_t y, Color color);
void set_verline(uint32_t x, uint32_t start, uint32_t end, Color color);
uint32_t pack_color(Color color);

#endif /* RENDER_H */
