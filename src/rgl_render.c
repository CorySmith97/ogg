
const char *model_vs =
    "#version 330 core\n"
    "layout(location = 0) in vec3 a_pos;\n"
    "layout(location = 1) in vec3 a_normal;\n"
    "layout(location = 2) in vec3 a_uv;\n"
    "uniform mat4 u_mvp;\n"
    "out vec3 v_normal;\n"
    "out vec2 v_uv;\n"
    "void main() {\n"
    "    v_normal = a_normal;\n"
    "    v_uv = a_uv.xy;\n"
    "    gl_Position = u_mvp * vec4(a_pos, 1.0);\n"
    "}\n";

const char *model_fs =
    "#version 330 core\n"
    "in vec3 v_normal;\n"
    "in vec2 v_uv;\n"
    "uniform sampler2D sampler;\n"
    "out vec4 frag_color;\n"
    "void main() {\n"
    "    frag_color = texture(sampler, v_uv);\n"
    "}\n";

const char *text_vs =
    "#version 330 core\n"
    "layout(location = 0) in vec2 a_pos;\n"
    "layout(location = 1) in vec2 a_uv;\n"
    "out vec2 v_uv;\n"
    "uniform vec2 u_screen;\n"
    "void main() {\n"
    "    v_uv = a_uv;\n"
    "    vec2 p = a_pos / u_screen * 2.0 - 1.0;\n"
    "    p.y = -p.y;\n"
    "    gl_Position = vec4(p, 0.0, 1.0);\n"
    "}\n";

const char *text_fs =
    "#version 330 core\n"
    "in vec2 v_uv;\n"
    "uniform sampler2D sampler;\n"
    "uniform vec4 u_color;\n"
    "out vec4 frag_color;\n"
    "void main() {\n"
    "    float d      = texture(sampler, v_uv).r;\n"
    "    float edge   = 0.502;\n"
    "    float spread = 0.047;\n"
    "    float alpha  = smoothstep(edge - spread, edge + spread, d);\n"
    "    if (alpha == 0.0) discard;\n"
    "    frag_color = vec4(u_color.rgb, u_color.a * alpha);\n"
    "}\n";

const char *shape_vs =
    "#version 330 core\n"
    "layout(location = 0) in vec3 a_pos;\n"
    "layout(location = 1) in vec4 a_color;\n"
    "out vec4 v_color;\n"
    "uniform vec2 u_screen;\n"
    "void main() {\n"
    "    v_color = a_color;\n"
    "    vec2 p = a_pos.xy / u_screen * 2.0 - 1.0;\n"
    "    p.y = -p.y;\n"
    "    gl_Position = vec4(p, 0.0, 1.0);\n"
    "}\n";

const char *shape_fs =
    "#version 330 core\n"
    "in vec4 v_color;\n"
    "out vec4 frag_color;\n"
    "void main() {\n"
    "   frag_color = v_color;"
    "}\n";

const char *shape3d_vs =
    "#version 330 core\n"
    "layout(location = 0) in vec3 a_pos;\n"
    "layout(location = 1) in vec4 a_color;\n"
    "out vec4 v_color;\n"
    "uniform mat4 u_mvp;\n"
    "void main() {\n"
    "    v_color = a_color;\n"
    "    gl_Position = u_mvp * vec4(a_pos, 1.0);\n"
    "}\n";

const char *tex2d_fs =
    "#version 330 core\n"
    "in vec2 v_uv;\n"
    "uniform sampler2D sampler;\n"
    "uniform vec4 u_tint;\n"
    "out vec4 frag_color;\n"
    "void main() {\n"
    "    frag_color = texture(sampler, v_uv) * u_tint;\n"
    "}\n";

const char *immediate_vs =
    "#version 330 core\n"
    "layout(location = 0) in vec3 a_pos;\n"
    "layout(location = 1) in vec3 a_normal;\n"
    "layout(location = 2) in vec2 a_uv;\n"
    "layout(location = 3) in vec4 a_color;\n"
    "uniform mat4 u_mvp;\n"
    "out vec3 v_normal;\n"
    "out vec2 v_uv;\n"
    "out vec4 v_color;\n"
    "void main() {\n"
    "    v_normal = a_normal;\n"
    "    v_uv = a_uv.xy;\n"
    "    v_color = a_color.xy;\n"
    "    gl_Position = u_mvp * vec4(a_pos, 1.0);\n"
    "}\n";

const char *immediate_fs =
    "#version 330 core\n"
    "in vec3 v_normal;\n"
    "in vec2 v_uv;\n"
    "uniform sampler2D sampler;\n"
    "out vec4 frag_color;\n"
    "void main() {\n"
    "    frag_color = texture(sampler, v_uv);\n"
    "}\n";

typedef struct {
    u32 shader_id;
    u32 vao;
    u32 triangle_count;
} DrawCall;

typedef struct {
    V2f pos;
    V2f uvs;
} TextVertex;

typedef struct {
    V3f position;
    V4f color;
} ShapeVertex;

typedef struct {
    const char *shader_name;
    u32         shader_id;
    u32         stride;
} ImmediateInfo;

typedef struct {
    u32 program;
    u32 vao;
    u32 vbo;
} Shader;

static u32 rgl_current_program = 0;

static struct {
    u32 shape_program;
    u32 shape_vao;
    u32 shape_vbo;
    u32 shape3d_program;
    u32 shape3d_vao;
    u32 shape3d_vbo;
    u32 text_program;
    u32 text_vao;
    u32 text_vbo;
    u32 tex2d_program;
    u32 model_program;
    u32 vao;
    u32 vbo;

    u32 immediate_program;
    u32 immediate_vao;
    u32 immediate_vbo;
    u32 fallback_texture;
} rgl;

static u32 rgl_compile_shader(u32 type, const char *src)
{
    u32 id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);
    return id;
}

static u32 rgl_link_program(u32 vs, u32 fs)
{
    u32 id = glCreateProgram();
    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return id;
}

void rgl_render_init(void)
{
    u32 vs = rgl_compile_shader(GL_VERTEX_SHADER, model_vs);
    u32 fs = rgl_compile_shader(GL_FRAGMENT_SHADER, model_fs);
    rgl.model_program = rgl_link_program(vs, fs);
    
    char log[512];
    glGetShaderInfoLog(vs, 512, NULL, log);
    if (log[0]) log_error("VS: %s\n", log);

    glGetShaderInfoLog(fs, 512, NULL, log);
    if (log[0]) log_error("FS: %s\n", log);

    glGenVertexArrays(1, &rgl.vao);
    glGenBuffers(1, &rgl.vbo);

    glBindVertexArray(rgl.vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(sizeof(V3f)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(sizeof(V3f) * 2));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    u32 tex_vs = rgl_compile_shader(GL_VERTEX_SHADER,   text_vs);
    u32 tex_fs = rgl_compile_shader(GL_FRAGMENT_SHADER, text_fs);
    rgl.text_program = rgl_link_program(tex_vs, tex_fs);

    glGenVertexArrays(1, &rgl.text_vao);
    glGenBuffers(1, &rgl.text_vbo);

    glBindVertexArray(rgl.text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.text_vbo);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(V2f) * 2, (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(V2f) * 2, (void *)(sizeof(V2f)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    u32 shapec_vs = rgl_compile_shader(GL_VERTEX_SHADER,   shape_vs);
    u32 shapec_fs = rgl_compile_shader(GL_FRAGMENT_SHADER, shape_fs);
    rgl.shape_program = rgl_link_program(shapec_vs, shapec_fs);

    glGenVertexArrays(1, &rgl.shape_vao);
    glGenBuffers(1, &rgl.shape_vbo);

    glBindVertexArray(rgl.shape_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.shape_vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex), (void *)(sizeof(V3f)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    u32 shape3d_vs_s = rgl_compile_shader(GL_VERTEX_SHADER,   shape3d_vs);
    u32 shape3d_fs_s = rgl_compile_shader(GL_FRAGMENT_SHADER, shape_fs);
    rgl.shape3d_program = rgl_link_program(shape3d_vs_s, shape3d_fs_s);

    glGenVertexArrays(1, &rgl.shape3d_vao);
    glGenBuffers(1, &rgl.shape3d_vbo);

    glBindVertexArray(rgl.shape3d_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.shape3d_vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex), (void *)(sizeof(V3f)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    u32 tex2d_vs_s = rgl_compile_shader(GL_VERTEX_SHADER,   text_vs);
    u32 tex2d_fs_s = rgl_compile_shader(GL_FRAGMENT_SHADER, tex2d_fs);
    rgl.tex2d_program = rgl_link_program(tex2d_vs_s, tex2d_fs_s);
    /* reuses text_vao / text_vbo — same TextVertex layout (V2f pos + V2f uv) */

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, platform_ctx.width, platform_ctx.height);

    u8 white[4] = {255, 255, 255, 255};
    glGenTextures(1, &rgl.fallback_texture);
    glBindTexture(GL_TEXTURE_2D, rgl.fallback_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
}

void set_shader(const char *name)
{
    (void)name;
    rgl_current_program = rgl.model_program;
    glUseProgram(rgl_current_program);
}

void set_uniform_v3f(const char *name, V3f data)
{
    s32 loc = glGetUniformLocation(rgl_current_program, name);
    glUniform3f(loc, data.x, data.y, data.z);
}

void set_uniform_mat4(const char *name, Mat4 data)
{
    s32 loc = glGetUniformLocation(rgl_current_program, name);
    glUniformMatrix4fv(loc, 1, GL_TRUE, data.c);
}

void rgl_renderer_flush(void)
{
}

void rgl_clear_background(Color color)
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void rgl_draw_model(Asset_Model *model, V3f position, Mat3 rotation, b32 selected)
{
    UNUSED(selected);

    Mat4 mdl = {
         rotation.c[0],  rotation.c[1],  rotation.c[2], position.x,
         rotation.c[3],  rotation.c[4],  rotation.c[5], position.y,
        -rotation.c[6], -rotation.c[7], -rotation.c[8], position.z,
         0,              0,              0,             1,
    };

    Mat4 view = camera_matrix(renderer.camera);

    f32 n = 0.1f, fr = 1000.0f;
    f32 ar = (f32)platform_ctx.width / (f32)platform_ctx.height;
    Mat4 proj = {
        1.0f/ar, 0,    0,               0,
        0,       1.0f, 0,               0,
        0,       0,    (n+fr)/(fr-n),   2.0f*fr*n/(n-fr),
        0,       0,    1,               0,
    };

    Mat4 mvp = mat4_mul(mat4_mul(proj, view), mdl);

    glUseProgram(rgl.model_program);

    s32 mvp_loc = glGetUniformLocation(rgl.model_program, "u_mvp");

    glUniformMatrix4fv(mvp_loc, 1, GL_TRUE, mvp.c);

    s32 count = arrlen(model->vertices);

    u32 tex_id = rgl.fallback_texture;
    if (model->mtl && model->mtl->diffuse_texture)
        tex_id = model->mtl->diffuse_texture->id;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glUniform1i(glGetUniformLocation(rgl.model_program, "sampler"), 0);

    glBindVertexArray(rgl.vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.vbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), model->vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, count);
    glBindVertexArray(0);
}

void rgl_draw_model_with_light(Asset_Model *model, V3f position, Mat3 rotation, Light light)
{
    (void)light;
    rgl_draw_model(model, position, rotation, 0);
}

void rgl_draw_model_textured(Asset_Model *model, V3f position, Mat3 rotation)
{
    rgl_draw_model(model, position, rotation, 0);
}

void rgl_draw_texture(Texture *tex, Recs32 rec)
{
    glUseProgram(rgl.tex2d_program);

    s32 screen_loc = glGetUniformLocation(rgl.tex2d_program, "u_screen");
    glUniform2f(screen_loc, (f32)platform_ctx.width, (f32)platform_ctx.height);

    s32 tint_loc = glGetUniformLocation(rgl.tex2d_program, "u_tint");
    glUniform4f(tint_loc, 1.0f, 1.0f, 1.0f, 1.0f);

    f32 x0 = (f32)rec.x,       y0 = (f32)rec.y;
    f32 x1 = (f32)(rec.x + rec.w), y1 = (f32)(rec.y + rec.h);

    TextVertex vertices[6] = {
        {.pos = v2f(x1, y1), .uvs = v2f(1, 1)},
        {.pos = v2f(x1, y0), .uvs = v2f(1, 0)},
        {.pos = v2f(x0, y0), .uvs = v2f(0, 0)},
        {.pos = v2f(x0, y1), .uvs = v2f(0, 1)},
        {.pos = v2f(x1, y1), .uvs = v2f(1, 1)},
        {.pos = v2f(x0, y0), .uvs = v2f(0, 0)},
    };

    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glUniform1i(glGetUniformLocation(rgl.tex2d_program, "sampler"), 0);
    glBindVertexArray(rgl.text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.text_vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(TextVertex), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void rgl_draw_text(Font *f, const char *str, V2i pos, f32 size, Color color)
{
    glUseProgram(rgl.text_program);

    u32 len   = strlen(str);
    u32 count = len * 6;

    f32 scale = size / f->size;  // ratio between desired size and baked size

    s32 screen_loc = glGetUniformLocation(rgl.text_program, "u_screen");
    glUniform2f(screen_loc, (f32)platform_ctx.width, (f32)platform_ctx.height);

    s32 color_loc = glGetUniformLocation(rgl.text_program, "u_color");
    glUniform4f(color_loc,
        color.r / 255.0f, color.g / 255.0f,
        color.b / 255.0f, color.a / 255.0f);

    TextVertex *vertices = NULL;
    f32 cursor_x = (f32)pos.x;
    f32 cursor_y = (f32)pos.y + f->ascent * scale;
    f32 cell     = f->glyphs['M'].advance * scale;  // fixed advance


    for (u32 i = 0; i < len; i++) {
        u8 c = (u8)str[i];
        GlyphInfo *g = &f->glyphs[c];

        // Skip glyphs with no visual (space, control chars)
        if (g->u0 == g->u1 || g->v0 == g->v1) {
            cursor_x += cell;
            continue;
        }

        // Screen-space quad — apply xoff/yoff so glyphs sit on the baseline
        f32 x0 = cursor_x + g->x_off * scale;
        f32 y0 = cursor_y + g->y_off * scale;
        f32 x1 = x0 + g->width  * scale;
        f32 y1 = y0 + g->height * scale;

        // UVs straight from the metrics — no bias needed for SDF
        f32 u0 = g->u0, v0 = g->v0;
        f32 u1 = g->u1, v1 = g->v1;

        // Triangle 1
        arrput(vertices, ((TextVertex){ .pos = {x1, y1}, .uvs = {u1, v1} }));
        arrput(vertices, ((TextVertex){ .pos = {x1, y0}, .uvs = {u1, v0} }));
        arrput(vertices, ((TextVertex){ .pos = {x0, y0}, .uvs = {u0, v0} }));
        // Triangle 2
        arrput(vertices, ((TextVertex){ .pos = {x0, y0}, .uvs = {u0, v0} }));
        arrput(vertices, ((TextVertex){ .pos = {x0, y1}, .uvs = {u0, v1} }));
        arrput(vertices, ((TextVertex){ .pos = {x1, y1}, .uvs = {u1, v1} }));

            cursor_x += cell;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, f->texture->id);
    glUniform1i(glGetUniformLocation(rgl.text_program, "sampler"), 0);

    glBindVertexArray(rgl.text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.text_vbo);
    glBufferData(GL_ARRAY_BUFFER, arrlen(vertices) * sizeof(TextVertex), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, arrlen(vertices));
    glBindVertexArray(0);
    arrfree(vertices);
}


void rgl_draw_string8(Font *f, String8 str, V2i pos, f32 size, Color color)
{
    glUseProgram(rgl.text_program);

    u32 len   = str.len;
    u32 count = len * 6;

    f32 scale = size / f->size;  // ratio between desired size and baked size

    s32 screen_loc = glGetUniformLocation(rgl.text_program, "u_screen");
    glUniform2f(screen_loc, (f32)platform_ctx.width, (f32)platform_ctx.height);

    s32 color_loc = glGetUniformLocation(rgl.text_program, "u_color");
    glUniform4f(color_loc,
        color.r / 255.0f, color.g / 255.0f,
        color.b / 255.0f, color.a / 255.0f);

    TextVertex *vertices = NULL;
    f32 cursor_x = (f32)pos.x;
    f32 cursor_y = (f32)pos.y + f->ascent * scale;
    f32 cell     = f->glyphs['M'].advance * scale;  // fixed advance

    for (u32 i = 0; i < len; i++) {
        u8 c = (u8)str.data[i];
        GlyphInfo *g = &f->glyphs[c];

        // Skip glyphs with no visual (space, control chars)
        if (g->u0 == g->u1 || g->v0 == g->v1) {
            cursor_x += cell;
            continue;
        }

        // Screen-space quad — apply xoff/yoff so glyphs sit on the baseline
        f32 x0 = cursor_x + g->x_off * scale;
        f32 y0 = cursor_y + g->y_off * scale;
        f32 x1 = x0 + g->width  * scale;
        f32 y1 = y0 + g->height * scale;

        // UVs straight from the metrics — no bias needed for SDF
        f32 u0 = g->u0, v0 = g->v0;
        f32 u1 = g->u1, v1 = g->v1;

        // Triangle 1
        arrput(vertices, ((TextVertex){ .pos = {x1, y1}, .uvs = {u1, v1} }));
        arrput(vertices, ((TextVertex){ .pos = {x1, y0}, .uvs = {u1, v0} }));
        arrput(vertices, ((TextVertex){ .pos = {x0, y0}, .uvs = {u0, v0} }));
        // Triangle 2
        arrput(vertices, ((TextVertex){ .pos = {x0, y0}, .uvs = {u0, v0} }));
        arrput(vertices, ((TextVertex){ .pos = {x0, y1}, .uvs = {u0, v1} }));
        arrput(vertices, ((TextVertex){ .pos = {x1, y1}, .uvs = {u1, v1} }));

        cursor_x += cell;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, f->texture->id);
    glUniform1i(glGetUniformLocation(rgl.text_program, "sampler"), 0);

    glBindVertexArray(rgl.text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.text_vbo);
    glBufferData(GL_ARRAY_BUFFER, arrlen(vertices) * sizeof(TextVertex), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, arrlen(vertices));
    glBindVertexArray(0);
    arrfree(vertices);
}


void rgl_draw_recs32(Recs32 rec, f32 z, Color color)  
{ 
    glUseProgram(rgl.shape_program);

    u32 count = 6;

    s32 screen_loc = glGetUniformLocation(rgl.shape_program, "u_screen");
    glUniform2f(screen_loc, (f32)platform_ctx.width, (f32)platform_ctx.height);

    V4f c = v4f(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);

    f32 x0 = rec.x;
    f32 y0 = rec.y;
    f32 x1 = rec.x + rec.w;
    f32 y1 = rec.y + rec.h;

    ShapeVertex vertices[6] = {
        {v3f(x1, y1, z), c},
        {v3f(x1, y0, z), c},
        {v3f(x0, y0, z), c},
        {v3f(x0, y1, z), c},
        {v3f(x1, y1, z), c},
        {v3f(x0, y0, z), c},
    };


    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(rgl.shape_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.shape_vbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(ShapeVertex), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, count);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void rgl_draw_texture_w_uvs(Texture *tex, Recs32 rec, V3f uvs[4], Color colors[4], u32 flags)
{
    (void)flags;
    glUseProgram(rgl.tex2d_program);

    s32 screen_loc = glGetUniformLocation(rgl.tex2d_program, "u_screen");
    glUniform2f(screen_loc, (f32)platform_ctx.width, (f32)platform_ctx.height);

    s32 tint_loc = glGetUniformLocation(rgl.tex2d_program, "u_tint");
    glUniform4f(tint_loc,
        colors[0].r / 255.0f, colors[0].g / 255.0f,
        colors[0].b / 255.0f, colors[0].a / 255.0f);

    f32 x0 = (f32)rec.x,           y0 = (f32)rec.y;
    f32 x1 = (f32)(rec.x + rec.w), y1 = (f32)(rec.y + rec.h);

    /* uvs: [0]=TL [1]=TR [2]=BR [3]=BL */
    TextVertex vertices[6] = {
        {.pos = v2f(x1, y1), .uvs = v2f(uvs[2].x, uvs[2].y)},
        {.pos = v2f(x1, y0), .uvs = v2f(uvs[1].x, uvs[1].y)},
        {.pos = v2f(x0, y0), .uvs = v2f(uvs[0].x, uvs[0].y)},
        {.pos = v2f(x0, y1), .uvs = v2f(uvs[3].x, uvs[3].y)},
        {.pos = v2f(x1, y1), .uvs = v2f(uvs[2].x, uvs[2].y)},
        {.pos = v2f(x0, y0), .uvs = v2f(uvs[0].x, uvs[0].y)},
    };

    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glUniform1i(glGetUniformLocation(rgl.tex2d_program, "sampler"), 0);
    glBindVertexArray(rgl.text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.text_vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(TextVertex), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void rgl_draw_rectangle3d(V3f bl, V3f br, V3f tl, V3f tr, Color color, u32 flags)
{
    (void)flags;

    Mat4 view = camera_matrix(renderer.camera);

    f32 n = 0.1f, fr = 1000.0f;
    f32 ar = (f32)platform_ctx.width / (f32)platform_ctx.height;
    Mat4 proj = {
        1.0f/ar, 0,    0,               0,
        0,       1.0f, 0,               0,
        0,       0,    (n+fr)/(fr-n),   2.0f*fr*n/(n-fr),
        0,       0,    1,               0,
    };

    Mat4 mvp = mat4_mul(proj, view);

    glUseProgram(rgl.shape3d_program);

    s32 mvp_loc = glGetUniformLocation(rgl.shape3d_program, "u_mvp");
    glUniformMatrix4fv(mvp_loc, 1, GL_TRUE, mvp.c);

    V4f c = v4f(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);

    ShapeVertex vertices[6] = {
        {tl, c},
        {tr, c},
        {br, c},
        {tl, c},
        {br, c},
        {bl, c},
    };

    glBindVertexArray(rgl.shape3d_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.shape3d_vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(ShapeVertex), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void rgl_draw_multitriangle(V3f v1, V3f v2, V3f v3, Color c1, Color c2, Color c3, u32 flags)
{
    
    glUseProgram(rgl.shape_program);

    s32 screen_loc = glGetUniformLocation(rgl.tex2d_program, "u_screen");
    glUniform2f(screen_loc, (f32)platform_ctx.width, (f32)platform_ctx.height);

    V4f color1 = v4f(c1.r / 255.0f, c1.g / 255.0f, c1.b / 255.0f, c1.a / 255.0f);
    V4f color2 = v4f(c2.r / 255.0f, c2.g / 255.0f, c2.b / 255.0f, c2.a / 255.0f);
    V4f color3 = v4f(c3.r / 255.0f, c3.g / 255.0f, c3.b / 255.0f, c3.a / 255.0f);

    ShapeVertex vertices[3] = {
        {v1, color1},
        {v2, color2},
        {v3, color3},
    };

    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(rgl.shape_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.shape_vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(ShapeVertex), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void rgl_draw_multitriangle3d(V3f v1, V3f v2, V3f v3, Color c1, Color c2, Color c3, u32 flags)
{
    (void)flags;

    Mat4 view = camera_matrix(renderer.camera);

    f32 n = 0.1f, fr = 1000.0f;
    f32 ar = (f32)platform_ctx.width / (f32)platform_ctx.height;
    Mat4 proj = {
        1.0f/ar, 0,    0,               0,
        0,       1.0f, 0,               0,
        0,       0,    (n+fr)/(fr-n),   2.0f*fr*n/(n-fr),
        0,       0,    1,               0,
    };

    Mat4 mvp = mat4_mul(proj, view);

    glUseProgram(rgl.shape3d_program);

    s32 mvp_loc = glGetUniformLocation(rgl.shape3d_program, "u_mvp");
    glUniformMatrix4fv(mvp_loc, 1, GL_TRUE, mvp.c);

    V4f color1 = v4f(c1.r / 255.0f, c1.g / 255.0f, c1.b / 255.0f, c1.a / 255.0f);
    V4f color2 = v4f(c2.r / 255.0f, c2.g / 255.0f, c2.b / 255.0f, c2.a / 255.0f);
    V4f color3 = v4f(c3.r / 255.0f, c3.g / 255.0f, c3.b / 255.0f, c3.a / 255.0f);

    ShapeVertex vertices[3] = {
        {v1, color1},
        {v2, color2},
        {v3, color3},
    };

    glBindVertexArray(rgl.shape3d_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.shape3d_vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(ShapeVertex), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void rgl_draw_triangle3d(V3f v1, V3f v2, V3f v3, Color color, u32 flags)
{
    (void)flags;

    Mat4 view = camera_matrix(renderer.camera);

    f32 n = 0.1f, fr = 1000.0f;
    f32 ar = (f32)platform_ctx.width / (f32)platform_ctx.height;
    Mat4 proj = {
        1.0f/ar, 0,    0,               0,
        0,       1.0f, 0,               0,
        0,       0,    (n+fr)/(fr-n),   2.0f*fr*n/(n-fr),
        0,       0,    1,               0,
    };

    Mat4 mvp = mat4_mul(proj, view);

    glUseProgram(rgl.shape3d_program);

    s32 mvp_loc = glGetUniformLocation(rgl.shape3d_program, "u_mvp");
    glUniformMatrix4fv(mvp_loc, 1, GL_TRUE, mvp.c);

    V4f c = v4f(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);

    ShapeVertex vertices[3] = {
        {v1, c},
        {v2, c},
        {v3, c},
    };

    glBindVertexArray(rgl.shape3d_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.shape3d_vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(ShapeVertex), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void rgl_draw_texture3d(Texture *tex, V3f bl, V3f br, V3f tl, V3f tr, Color color, u32 flags) 
{

    Mat4 view = camera_matrix(renderer.camera);

    f32 n = 0.1f, fr = 1000.0f;
    f32 ar = (f32)platform_ctx.width / (f32)platform_ctx.height;
    Mat4 proj = {
        1.0f/ar, 0,    0,               0,
        0,       1.0f, 0,               0,
        0,       0,    (n+fr)/(fr-n),   2.0f*fr*n/(n-fr),
        0,       0,    1,               0,
    };

    Mat4 mvp = mat4_mul(proj, view);

    glUseProgram(rgl.model_program);

    s32 mvp_loc = glGetUniformLocation(rgl.model_program, "u_mvp");

    glUniformMatrix4fv(mvp_loc, 1, GL_TRUE, mvp.c);

    u32 tex_id = tex->id;

    // Tri 1: TL, BR, BL
    V3f uvs1[3] = {
        {0.0f, 0.0f, 1.0f}, // TL
        {1.0f, 0.0f, 1.0f}, // TR
        {1.0f, 1.0f, 1.0f}, // BR
    };
    // Tri 2: TL, TR, BR
    V3f uvs2[3] = {
        {0.0f, 0.0f, 1.0f}, // TL
        {1.0f, 1.0f, 1.0f}, // BR
        {0.0f, 1.0f, 1.0f}, // BL
    };

    Vertex vertices[6] = {
        {.position = tl, .uv = uvs1[0]},
        {.position = tr, .uv = uvs1[1]},
        {.position = br, .uv = uvs1[2]},
        {.position = tl, .uv = uvs2[0]},
        {.position = br, .uv = uvs2[1]},
        {.position = bl, .uv = uvs2[2]},
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glUniform1i(glGetUniformLocation(rgl.model_program, "sampler"), 0);

    glDisable(GL_BLEND);
    glBindVertexArray(rgl.vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

f32 *immediate_vertex = NULL;

// This function sets the way the renderer should interpret the render buffer data
// in addition to which actual shader should be used.
void rgl_immediate_set_shader(String8 name)
{
}

void rgl_immediate_push_v(V3f v, Color c)
{
    f32 cr = (f32)c.r / 255.0;
    f32 cg = (f32)c.g / 255.0;
    f32 cb = (f32)c.b / 255.0;
    f32 ca = (f32)c.a / 255.0;

    arrput(immediate_vertex, v.x);
    arrput(immediate_vertex, v.y);
    arrput(immediate_vertex, v.z);
    arrput(immediate_vertex, cr);
    arrput(immediate_vertex, cg);
    arrput(immediate_vertex, cb);
    arrput(immediate_vertex, ca);
}

void rgl_immediate_flush(void)
{
    Mat4 view = camera_matrix(renderer.camera);

    f32 n = 0.1f, fr = 1000.0f;
    f32 ar = (f32)platform_ctx.width / (f32)platform_ctx.height;
    Mat4 proj = {
        1.0f/ar, 0,    0,               0,
        0,       1.0f, 0,               0,
        0,       0,    (n+fr)/(fr-n),   2.0f*fr*n/(n-fr),
        0,       0,    1,               0,
    };

    Mat4 mvp = mat4_mul(proj, view);

    glUseProgram(rgl.shape3d_program);

    s32 mvp_loc = glGetUniformLocation(rgl.shape3d_program, "u_mvp");

    glUniformMatrix4fv(mvp_loc, 1, GL_TRUE, mvp.c);

    glBindVertexArray(rgl.shape3d_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rgl.shape3d_vbo);
    glBufferData(GL_ARRAY_BUFFER, arrlen(immediate_vertex) * sizeof(f32), immediate_vertex, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, arrlen(immediate_vertex) / 7);
    glBindVertexArray(0);

    arrsetlen(immediate_vertex, 0);
}
