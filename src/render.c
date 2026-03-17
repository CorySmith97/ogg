#include "render.h"

void 
render_init(void)
{
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

Color
color_add(Color c1, Color c2)
{
    return (Color){
        c1.r + c2.r,
        c1.g + c2.g,
        c1.b + c2.b,
        c1.a + c2.a,
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
    if (v1.x < 0 || v1.x > WIDTH 
        || v2.x < 0 || v2.x > WIDTH 
        || v3.x < 0 || v3.x > WIDTH 
        || v1.y < 0 || v1.y > HEIGHT 
        || v2.y < 0 || v2.y > HEIGHT 
        || v3.y < 0 || v3.y > HEIGHT) return;

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

void 
set_triangle_multicolor(V2i v1, V2i v2, V2i v3, Color c1, Color c2, Color c3)
{
    if (v1.x < 0 || v1.x > WIDTH 
        || v2.x < 0 || v2.x > WIDTH 
        || v3.x < 0 || v3.x > WIDTH 
        || v1.y < 0 || v1.y > HEIGHT 
        || v2.y < 0 || v2.y > HEIGHT 
        || v3.y < 0 || v3.y > HEIGHT) return;
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


    for (int x = rec.min.x; x < rec.max.x; x++) {
        for (int y = rec.min.y; y < rec.max.y; y++) {
            if (barycentric(pos1, pos2, pos3, v2i(x, y), &bary)) {
                double z = (bary.x * v1.z + bary.y * v2.z + bary.z * v3.z);
                //logger(LOG_DEBUG, "z: %c, zbuf: %f", z, renderer.zbuffer[y][x]);
                if (z <= renderer.zbuffer[y][x]) continue;
                renderer.zbuffer[y][x] = z;
                // normalize z to 0.0 - 1.0
                float t = (z - NEAR) / (FAR - NEAR);
                t = fmaxf(0.0f, fminf(1.0f, t)); // clamp

                unsigned char c = (unsigned char)(t * 255.0f);
                
                set_pixel((uint32_t)x, (uint32_t)y, color); // (Color){c, c, c, 255});
            }
        }
    }
}

void 
set_triangle_3d_multicolor(V3f v1, V3f v2, V3f v3, Color c1, Color c2, Color c3)
{
    UNUSED(v1);
    UNUSED(v2);
    UNUSED(v3);
    UNUSED(c1);
    UNUSED(c2);
    UNUSED(c3);
}

// Top right, bottom right, top left, bottom left
void 
set_quad(V2i v1, V2i v2, V2i v3, V2i v4, Color c)
{
    set_triangle(v1, v2, v3, c);
    set_triangle(v1, v3, v4, c);
}

Color
get_color_from_image(Image *i, V2f uv)
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

void
draw_textured_triangle(Image *t, TextVertex v1, TextVertex v2, TextVertex v3)
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

}

/* void 
draw_textured_quad(TextVertex v1, TextVertex v2, TextVertex v3, TextVertex v4, Image *i)
{
    set_triangle(v1, v2, v3, c);
    set_triangle(v1, v3, v4, c);
} */

/* void
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
} */

void
clear_background(void)
{
    memset(renderer.pixels, 0, WIDTH * HEIGHT * sizeof(uint32_t));

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            renderer.zbuffer[y][x] = -1e10;
        }
    }
}

