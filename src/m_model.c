void model_editor_init(void)
{
    m_editor.arena     = arena_alloc();
    m_editor.transform = mat3_identity();
    m_editor.camera.front = v3f_normalize(
        v3f_sub(m_editor.camera.target, m_editor.camera.position));
}

void model_editor_load_model(String8 model_name)
{
    char *str = str8_to_cstring(m_editor.arena, model_name);
    m_editor.selected = get_model(str);
    console_write_log_alloc("Loaded model %s", str);

    u32 vert_count = (u32)arrlen(m_editor.selected->vertices);
    m_editor.selected_triangles = push_array(m_editor.arena, b32, vert_count);
    memset(m_editor.selected_triangles, 0, sizeof(b32) * vert_count);

}

void model_editor_frame(void)
{
    if (!m_editor.selected) return;

    V2f mouse       = get_mouse_pos();
    V2f mouse_delta = get_mouse_delta();
    Ray m_ray       = get_mouse_ray(m_editor.camera, mouse);

    draw_model_triangle_selection(m_editor.selected, v3f(0,0,0),
                                  m_editor.transform, m_editor.selected_triangles);
}

