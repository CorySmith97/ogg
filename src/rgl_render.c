#include <Opengl/gl.h>

typedef struct {
    u32 shader_id;
    u32 vao;
    u32 triangle_count;
} DrawCall;

void rgl_render_init(void)
{
}

void set_shader(String8 name);
void set_uniform_v3f(const char *name, V3f data);
void set_uniform_mat4(const char *name, Mat4 data);

void renderer_flush(void)
{
}

void clear_background(Color color)
{
    glClear(color.r/255, color.g/255, color.b/255, color.a/255);
}


void draw_model(Asset_Model *model, V3f position, Mat3 rotation, b32 selected)
{

}

void draw_model_with_light(Asset_Model *model, V3f position, Mat3 rotation, Light light)
{
}

void draw_model_textured(Asset_Model *model, V3f position, Mat3 rotation);
void draw_texture(Texture *tex, Recs32 rec);
void draw_text(Font *f, const char *str, V2i pos, f32 size, Color color);
void draw_recs32(Recs32 rec, f32 z, Color color);
void draw_texture_w_uvs(Texture *tex, Recs32 rec, V3f uvs[4], Color colors[4], u32 flags);
void draw_rectangle3d(V3f bl, V3f br, V3f tl, V3f tr, Color color, u32 flags);
void draw_triangle3d(V3f v1, V3f v2, V3f v3, Color color, u32 flags);
void draw_texture3d(Texture *tex, V3f bl, V3f br, V3f tl, V3f tr, Color color, u32 flags);
