#include "render.h"

void 
render_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != SDL_FALSE) {
        printf("Failed to initial Video\n");
        exit(EXIT_FAILURE);
    }

    renderer.window = SDL_CreateWindow("Hello", 
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
            WIDTH, HEIGHT, 
            SDL_WINDOW_SHOWN);


    renderer.renderer = SDL_CreateRenderer(renderer.window, 0, SDL_RENDERER_SOFTWARE);
    renderer.texture = SDL_CreateTexture(renderer.renderer, 
            SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 
            WIDTH, HEIGHT);
}

uint32_t 
pack_color(struct Color color)
{
    return ((uint32_t)color.a << 24) |
           ((uint32_t)color.b << 16) |
           ((uint32_t)color.g << 8)  |
           ((uint32_t)color.r);
}

Color
color_scale(Color c, double value)
{
    return (struct Color){
        c.r * value,
        c.g * value,
        c.b * value,
        c.a,
    };
}


void
set_pixel(uint32_t x, uint32_t y, Color color) 
{
    if (x >= (uint32_t)renderer.width 
            || x < 0 
            || y >= (uint32_t)renderer.height 
            || y < 0) return;

    renderer.pixels[y][x] = pack_color(color);
}

void
set_verline(uint32_t x,  uint32_t start, uint32_t end, Color color)
{
    for (uint32_t i = start; i < end; i++) {
        set_pixel(x, i, color);
    }
}

void 
set_line(V2i v, V2i u, Color color)
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

void 
set_triangle(V2i v1, V2i v2, V2i v3, Color color)
{
    AABBi rec = {
        v2i(min(v1.x, min(v2.x, v3.x)), min(v1.y, min(v2.y, v3.y))),
        v2i(max(v1.x, max(v2.x, v3.x)), max(v1.y, max(v2.y, v3.y))),
    };
    V3f bary;

    for (int x = rec.min.x; x < rec.max.x; x++) {
        for (int y = rec.min.y; y < rec.max.y; y++) {
            if (barycentric(v1, v2, v3, v2i(x, y), &bary)) {
                set_pixel((uint32_t)x, (uint32_t)y, color);
            }
        }
    }
}

void set_triangle_3d(V3f v1, V3f v2, V3f v3, Color color)
{
    V2i pos1 = to_screen(project(v1));
    V2i pos2 = to_screen(project(v2));
    V2i pos3 = to_screen(project(v3));

    set_triangle(pos1, pos2, pos3, color);
}


void
present(void) 
{
    void *pixels;
    int pitch;
    SDL_LockTexture(renderer.texture, NULL, &pixels, &pitch);
    memcpy(pixels, renderer.pixels, renderer.width * renderer.height * sizeof(uint32_t));
    //printf("pixel 0: %d Pixel 100: %d\n", s->pixels[0], s->pixels[100]);
    SDL_UnlockTexture(renderer.texture);

    SDL_RenderClear(renderer.renderer);
    SDL_RenderCopy(renderer.renderer, renderer.texture, NULL, NULL);
    SDL_RenderPresent(renderer.renderer);
}

void
clear_background(void)
{
    memset(renderer.pixels, 0, WIDTH * HEIGHT * sizeof(uint32_t));
}

