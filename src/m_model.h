#ifndef MODEL_EDITOR
#define MODEL_EDITOR

typedef enum {
    MODEL_EDITOR_MODE_SELECT,   // existing triangle selection behaviour
    MODEL_EDITOR_MODE_PAINT,    // vertex group painting
    MODEL_EDITOR_MODE_RIG,      // place/move/parent joints, auto-skin
    MODEL_EDITOR_MODE_ANIM,     // gizmo-based keyframe posing
} ModelEditorMode;

// One distinct color per group index (0-15), used both in the viewport and the UI palette.
static const Color GROUP_PALETTE[16] = {
    {255,  80,  80, 200}, {80,  255,  80, 200}, { 80,  80, 255, 200}, {255, 255,  80, 200},
    {255,  80, 255, 200}, {80,  255, 255, 200}, {255, 180,  80, 200}, {180,  80, 255, 200},
    { 80, 180,  80, 200}, {80,  130, 255, 200}, {255, 130, 130, 200}, {130, 255, 130, 200},
    {130, 130, 255, 200}, {255, 220, 130, 200}, {130, 220, 255, 200}, {220, 130, 255, 200},
};

static struct {
    Arena           *arena;
    Asset_Model     *selected;
    b32             *selected_triangles;
    Mat3             transform;
    b32              mouse_selecting;
    V2f              min;
    V2f              max;
    Camera           camera;
    Font            *hud_font;

    // Skin painting
    ModelEditorMode  mode;
    u16              active_group;   // group currently being painted (0-15)
    s32              hovered_tri;    // flat vertex index of the triangle under the cursor, -1 = none
    V2f              last_pick_mouse; // mouse position at the last pick_triangle call
    V3f             *world_positions; // pre-baked world-space positions [arrlen(vertices)], parallel to model->vertices
    char             model_name[256]; // asset-cache key of the loaded model, used for skin_save path

    // AI-GENERATED: anim editor state (used only in MODEL_EDITOR_MODE_ANIM).
    // Review: these fields are only valid after "Build Skel" is pressed.
    FrameBase       *edit_base;         // flat FrameBase auto-built from painted groups
    AnimData        *edit_anim;         // AnimData accumulating recorded keyframes
    Transform        edit_pose[16];     // live per-joint transform (indexed by group 0-15)
    V3f              joint_centers[16]; // world-space centroids of each group's base positions
    b32              joint_has_verts[16]; // true if any base position belongs to this group
    Gizmo            joint_gizmo;       // single shared gizmo, repositioned per joint selection
    s32              selected_joint;    // group index of joint currently being posed (-1 = none)
    s32              gizmo_axis;        // active gizmo axis index (-1 = none)
    char             seq_name[32];      // sequence name for the saved .anim file
    float            frame_dur_ms;      // ms per frame when saving (mu_Real = float)
    s32              selected_frame;    // index of frame selected in the frame list (-1 = none)

    // Box painting
    b32              box_painting;      // true while the user is drag-painting a screen rect
    V2f              box_paint_start;   // screen-space anchor of the box paint drag

    // Node dragging / IK
    b32              node_dragging;     // true while dragging a joint node directly
    b32              ik_mode;           // when true, dragging a node also rotates its parent
    b32              prefer_gizmo;      // when true, node clicks select the joint but don't activate free-drag
    V3f              ik_drag_target;    // world-space target accumulated during this node drag

    // Rig editor
    b32              joint_active[16];  // true for each joint explicitly placed in rig mode

} m_editor = {
    .selected           = NULL,
    .selected_triangles = NULL,
    .mode               = MODEL_EDITOR_MODE_SELECT,
    .active_group       = 0,
    .hovered_tri        = -1,
    .selected_joint     = -1,
    .gizmo_axis         = -1,
    .selected_frame     = -1,
    .seq_name           = "idle",
    .frame_dur_ms       = 200.0f,
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
void model_editor_rig_frame(f32 dt);

#endif // MODEL_EDITOR
