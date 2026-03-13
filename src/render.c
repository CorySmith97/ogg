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
    if (x > renderer.width || x < 0 || y > renderer.height || y < 0) return;

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

