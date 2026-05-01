#ifndef EDITOR_H
#define EDITOR_H

static struct {
    Arena  *arena;
    Camera camera;
    s32    selected_entity;
    s32    selected_axis;
    Gizmo  *gizmo;
    f32    camera_speed;

    f32    mouse_scroll;
    V2f    mouse_pos;
    V2f    mouse_delta;
    Ray    mouse_ray;
    b32    mouse_pressed;
    b32    mouse_down;
    b32    mouse_released;
    b32    tiles_only;
    b32    clicked_gizmo;
    b32    clicked_entity;
} editor = {
    .camera = {
        .target = {0, 0, 0},
        .position = {0,2,-2},
        .up = {0, 1, 0},
        .pitch = 45.0f,
        .yaw = 45.0f,
        .distance = 10.0f,
        .fovy = 180.0f,
    },
    .camera_speed = 0.2,
    .selected_entity = -1,
    .selected_axis   = -1,
};


void editor_init(void);
void editor_update(void);
void editor_draw(void);
void editor_camera_update(void);

#endif // EDITOR_H
