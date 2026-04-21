
// Internal Function Declarations
void set_verline(u32 x, u32 start, u32 end, Color color);
void set_line(V2i v, V2i u, Color color);
void set_triangle(V2i v1, V2i v2, V2i v3, Color color);
void set_triangle_multicolor(V2i v1, V2i v2, V2i v3, Color c1, Color c2, Color c3);
void set_triangle_3d(V3f v1, V3f v2, V3f v3, Color color);
void set_triangle_3d_multicolor(V3f v1, V3f v2, V3f v3, Color c1, Color c2, Color c3);
void set_quad(V2i v1, V2i v2, V2i v3, V2i v4, Color c);
Color get_color_from_texture(Texture *t, V2f uv);


#define PIXEL_INDEX(x, y) ((size_t)(x) + (size_t)(y) * (size_t)renderer.width)

static inline void set_pixel(s32 x, s32 y, Color src)
{
    if (x < 0 || y < 0 || x >= renderer.width || y >= renderer.height)
        return;

    size_t idx = PIXEL_INDEX(x, y);
    Color dst = (Color)renderer.pixels[idx];

    if (src.a == 0)
        return;

    if (src.a == 255) {
        renderer.pixels[idx] = src.rgba;
        return;
    }

    Color out = alpha_blend(src, dst);
    renderer.pixels[idx] = out.rgba;
}

void set_verline(u32 x, u32 start, u32 end, Color color)
{
    for (u32 i = start; i < end; i++)
    {
        set_pixel(x, i, color);
    }
}

void set_line(V2i v, V2i u, Color color)
{
    bool steep = abs(u.y - v.y) > abs(u.x - v.x);

    if (steep) {
        // Swap x and y within each point
        s32 tmp;
        tmp = v.x; v.x = v.y; v.y = tmp;
        tmp = u.x; u.x = u.y; u.y = tmp;
    }

    // Ensure left-to-right traversal
    if (v.x > u.x) {
        V2i tmp = v; v = u; u = tmp;
    }

    s32 dx = u.x - v.x;
    s32 dy = abs(u.y - v.y);
    s32 ystep = (v.y < u.y) ? 1 : -1;
    s32 ierror = 0;
    s32 y = v.y;

    for (s32 x = v.x; x <= u.x; x++)
    {
        if (steep)
            set_pixel(y, x, color);
        else
            set_pixel(x, y, color);

        ierror += dy;
        if (2 * ierror >= dx) {
            y += ystep;
            ierror -= dx;
        }
    }
}

bool check_bounds(V2i v1, V2i v2, V2i v3)
{
    if (v1.x < 0 || v1.x > GAME_WIDTH || v2.x < 0 || v2.x > GAME_WIDTH || v3.x < 0 || v3.x > GAME_WIDTH || v1.y < 0 || v1.y > GAME_HEIGHT || v2.y < 0 || v2.y > GAME_HEIGHT || v3.y < 0 || v3.y > GAME_HEIGHT)
        return false;

    return true;
}

void set_triangle(V2i v1, V2i v2, V2i v3, Color color)
{
    AABBi rec = {
        v2i(min(v1.x, min(v2.x, v3.x)), min(v1.y, min(v2.y, v3.y))),
        v2i(max(v1.x, max(v2.x, v3.x)), max(v1.y, max(v2.y, v3.y))),
    };
    V3f bary;

    for (s32 x = rec.min.x; x < rec.max.x; x++)
    {
        for (s32 y = rec.min.y; y < rec.max.y; y++)
        {
            if (barycentric(v1, v2, v3, v2i(x, y), &bary))
            {
                set_pixel((u32)x, (u32)y, color);
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

    for (s32 x = rec.min.x; x < rec.max.x; x++)
    {
        for (s32 y = rec.min.y; y < rec.max.y; y++)
        {
            if (barycentric(v1, v2, v3, v2i(x, y), &bary))
            {
                Color color = color_add(color_add(color_scale(c1, bary.x), color_scale(c2, bary.y)), color_scale(c3, bary.z));
                set_pixel((u32)x, (u32)y, color);
            }
        }
    }
}

void set_triangle_3d(V3f v1, V3f v2, V3f v3, Color color)
{
    V2i pos1 = to_screen(project(v1));
    V2i pos2 = to_screen(project(v2));
    V2i pos3 = to_screen(project(v3));

    AABBi rec = {
        v2i(min(pos1.x, min(pos2.x, pos3.x)), min(pos1.y, min(pos2.y, pos3.y))),
        v2i(max(pos1.x, max(pos2.x, pos3.x)), max(pos1.y, max(pos2.y, pos3.y))),
    };
    V3f bary;

    double total_area = signed_area(pos1.x, pos1.y, pos2.x, pos2.y, pos3.x, pos3.y);
    if (total_area < -1e6)
        return;

    for (s32 y = rec.min.y; y < rec.max.y; y++)
    {
        for (s32 x = rec.min.x; x < rec.max.x; x++)
        {
            if (barycentric(pos1, pos2, pos3, v2i(x, y), &bary))
            {
                double z = (bary.x * v1.z + bary.y * v2.z + bary.z * v3.z);
                // logger(LOG_DEBUG, "z: %c, zbuf: %f", z, renderer.zbuffer[y][x]);
                if (z < renderer.zbuffer[x + y * GAME_WIDTH])
                    continue;
                renderer.zbuffer[x + y * GAME_WIDTH] = z;

                set_pixel((u32)x, (u32)y, color); // (Color){c, c, c, 255});
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

// TODO there is some speed up that could be done here. SIMD
Color get_color_from_texture(Texture *t, V2f uv)
{
    uv.x = fmaxf(0.0f, fminf(1.0f, uv.x));
    uv.y = fmaxf(0.0f, fminf(1.0f, uv.y));

    size_t x = (size_t)(uv.x * (t->width  - 1));
    size_t y = (size_t)(uv.y * (t->height - 1));

    size_t idx = (y * t->width + x) * t->stride;

    Color c;
    c.r = t->data[idx + 0];
    c.g = t->data[idx + 1];
    c.b = t->data[idx + 2];
    // TODO add ability to have clear parts from a texture come through
    c.a = t->stride == 4 ? t->data[idx + 3] : 255;
    return c;
}


static inline f32 clamp01(f32 x)
{
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}

static inline f32 smoothstepf(f32 a, f32 b, f32 x)
{
    f32 t = clamp01((x - a) / (b - a));
    return t * t * (3.0f - 2.0f * t);
}

// sdf_sample_u8 is usually the red channel from the font atlas.
// 128 is the contour. spread controls softness.
// A good starting value is 8.0f to 16.0f depending on the atlas.
static inline u8 sdf_to_alpha(u8 sdf_sample_u8, f32 spread)
{
    f32 d = (f32)sdf_sample_u8 / 255.0f;   // 0..1
    f32 edge = 128.0f / 255.0f;            // ~0.502

    // spread is in normalized texture-value space
    f32 a = smoothstepf(edge - spread, edge + spread, d);
    s32 out = (s32)(a * 255.0f + 0.5f);
    if (out < 0) out = 0;
    if (out > 255) out = 255;
    return (u8)out;
}
