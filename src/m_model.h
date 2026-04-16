#ifndef MODEL_EDITOR
#define MODEL_EDITOR

static struct {
    // ---- existing ----
    Arena       *arena;
    Asset_Model *selected;
    b32         *selected_triangles;
    Mat3         transform;
    b32          mouse_selecting;
    V2f          min;
    V2f          max;
    Camera       camera;
    Font        *hud_font;

} m_editor = {
    .selected      = NULL,
    .selected_triangles = NULL,
    .camera = {
        .target   = {0, 0, 0},
        .position = {0, 2, -2},
        .up       = {0, 1, 0},
        .pitch    = 45.0f,
        .yaw      = 45.0f,
        .distance = 10.0f,
        .fovy     = 60.0f,
    },
};

void model_editor_load_model(String8 model_name);
void model_editor_init(void);
void model_editor_frame(void);
void model_editor_anim_frame(f32 dt);

#endif // MODEL_EDITOR
