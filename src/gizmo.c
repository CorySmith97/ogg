#include "gizmo.h"

void gizmo_draw(Gizmo *gizmo)
{
    float x = gizmo->position.x;
    float y = gizmo->position.y;
    float z = gizmo->position.z;
    float len = 1.0f;
    float o = 0.05f; // small offset along axis
    float t   = 0.02f; // half-thickness

    for (size_t i = 0; i < GIZMO_AXIS_COUNT; i++) {
        Color color;
        switch (i) {
            case GIZMO_AXIS_X:
                color = COLOR_RED;
                draw_rectangle3d(
                        v3f(x + o,       y - t, z),
                        v3f(x + len + o, y - t, z),
                        v3f(x + o,       y + t, z),
                        v3f(x + len + o, y + t, z),
                        color);
                break;

            case GIZMO_AXIS_Y:
                color = COLOR_BLUE;
                draw_rectangle3d(
                        v3f(x - t, y + o,       z),
                        v3f(x + t, y + o,       z),
                        v3f(x - t, y + len + o, z),
                        v3f(x + t, y + len + o, z),
                        color);
                break;

            case GIZMO_AXIS_Z:
                color = COLOR_GREEN;
                draw_rectangle3d(
                        v3f(x - t, y, z + o),
                        v3f(x + t, y, z + o),
                        v3f(x - t, y, z + len + o),
                        v3f(x + t, y, z + len + o),
                        color);
                break;
        }
    }
}

void gizmo_translation_modify(Gizmo *gizmo, Gizmo_Axis axis, V3f delta);
void gizmo_rotation_modify(Gizmo *gizmo, Gizmo_Axis axis, f32 angle);
Mat3 gizmo_get_rotation(Gizmo *gizmo);
