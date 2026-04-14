
void model_editor_load_model(String8 model_name)
{
    char *str = str8_to_cstring(m_editor.arena, model_name);
    m_editor.selected = get_model(str);
    console_write_log_alloc("Loaded model %s", str);
}

void model_editor_init(void)
{
    m_editor.arena = arena_alloc();
}

void model_editor_frame(void)
{
    if (!m_editor.selected) return;
    if (is_key_pressed(KEY_B)) {
    }

    draw_model(m_editor.selected, v3f(0,0,0), m_editor.transform, false);
}
