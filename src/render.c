#include "rs_render.c"
#include "rgl_render.c"

void console_render_swap(String8 params)
{
    if (str8_compare(params, str8_lit("opengl"))) {
        log_info("params: %d", params.len);
        renderer.backend = BACKEND_OPENGL;
        return;
    } else if (str8_compare(params, str8_lit("software"))) {
        renderer.backend = BACKEND_SOFTWARE;
        return;
    }
}

void render_init(void)
{
    rs_render_init();
    rgl_render_init();
}

void render_shutdown(void)
{
    rs_render_shutdown();
}

//
// Clear all the triangle buffer and push pixels to render buffer
//
void renderer_flush(void)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_renderer_flush();
        break;
        case BACKEND_OPENGL:
            rgl_renderer_flush();
        break;
    }
}
void clear_background(Color color)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_clear_background(color);
        break;
        case BACKEND_OPENGL:
            rgl_clear_background(color);
        break;
    }
}

void change_camera(Camera *camera)
{
    renderer.camera = camera;
}

void draw_model(Asset_Model *model, V3f position, Mat3 rotation, b32 selected)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_draw_model(model, position, rotation, selected);
        break;
        case BACKEND_OPENGL:
            rgl_draw_model(model, position, rotation, selected);
        break;
    }
}

void draw_model_with_light(Asset_Model *model, V3f position, Mat3 rotation, Light light)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_draw_model_with_light(model, position, rotation, light);
        break;
        case BACKEND_OPENGL:
            rgl_draw_model_with_light(model, position, rotation, light);
        break;
    }
}

void draw_model_triangle_selection(Asset_Model *model, V3f position, Mat3 rotation, b32 *selected)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_draw_model_triangle_selection(model, position, rotation, selected);
            break;
        case BACKEND_OPENGL:
            break;
    }
}

void draw_texture(Texture *tex, Recs32 rec)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_draw_texture(tex, rec);
        break;
        case BACKEND_OPENGL:
            rgl_draw_texture(tex, rec);
        break;
    }
}

void draw_text(Font *f, const char *str, V2i pos, f32 size, Color color)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_draw_text(f, str, pos, size, color);
        break;
        case BACKEND_OPENGL:
            rgl_draw_text(f, str, pos, size, color);
        break;
    }
}

void draw_string8(Font *f, String8 str, V2i pos, f32 size, Color color)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_draw_string8(f, str, pos, size, color);
        break;
        case BACKEND_OPENGL:
            rgl_draw_string8(f, str, pos, size, color);
        break;
    }
}

void draw_recs32(Recs32 rec, f32 z, Color color)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_draw_recs32(rec, z, color);
        break;
        case BACKEND_OPENGL:
            rgl_draw_recs32(rec, z, color);
        break;
    }
}

void draw_texture_w_uvs(Texture *tex, Recs32 rec, V3f uvs[4], Color colors[4], u32 flags)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_draw_texture_w_uvs(tex, rec, uvs, colors, flags);
        break;
        case BACKEND_OPENGL:
            rgl_draw_texture_w_uvs(tex, rec, uvs, colors, flags);
        break;
    }
}

void draw_rectangle3d(V3f bl, V3f br, V3f tl, V3f tr, Color color, u32 flags)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_draw_rectangle3d(bl, br, tl, tr, color, flags);
        break;
        case BACKEND_OPENGL:
            rgl_draw_rectangle3d(bl, br, tl, tr, color, flags);
        break;
    }
}

void draw_triangle3d(V3f v1, V3f v2, V3f v3, Color color, u32 flags)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_draw_triangle3d(v1, v2, v3, color, flags);
        break;
        case BACKEND_OPENGL:
            rgl_draw_triangle3d(v1, v2, v3, color, flags);
        break;
    }
}

void draw_texture3d(Texture *tex, V3f bl, V3f br, V3f tl, V3f tr, Color color, u32 flags)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_draw_texture3d(tex, bl, br, tl, tr, color, flags);
        break;
        case BACKEND_OPENGL:
            rgl_draw_texture3d(tex, bl, br, tl, tr, color, flags);
        break;
    }
}

void immediate_flush(void)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_immediate_flush();
        break;
        case BACKEND_OPENGL:
        break;
    }
}

void immediate_push_v(V3f v1, Color c)
{
    switch (renderer.backend) {
        case BACKEND_SOFTWARE:
            rs_immediate_push_v(v1, c);
        break;
        case BACKEND_OPENGL:
        break;
    }
}
