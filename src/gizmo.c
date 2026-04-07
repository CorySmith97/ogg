#include "gizmo.h"

#include "gizmo.h"

void gizmo_draw(Gizmo *gizmo)
{
    float x = gizmo->position.x;
    float y = gizmo->position.y;
    float z = gizmo->position.z;

    float len = 1.0f;
    float o   = 0.05f; // small offset along axis
    float po   = 0.3f; // small offset along axis
    float ps   = 0.10f; // small offset along axis
    float t   = 0.06f; // half-thickness
    float ah  = 0.12f; // arrowhead length
    float aw  = 0.06f; // arrowhead half-width

    for (size_t i = 0; i < GIZMO_AXIS_COUNT; i++) {
        Color color;

        switch (i) {
            case GIZMO_AXIS_X:
                color = COLOR_RED;

                // Shaft
                draw_rectangle3d(
                    v3f(x + o,           y - t, z),
                    v3f(x + len - ah,    y - t, z),
                    v3f(x + o,           y + t, z),
                    v3f(x + len - ah,    y + t, z),
                    color, TRIANGLE_WRITE_OVER_Z
                );

                // Arrowhead
                draw_triangle3d(
                    v3f(x + len + o,     y,      z),
                    v3f(x + len - ah + o, y - aw, z),
                    v3f(x + len - ah + o, y + aw, z),
                    color, TRIANGLE_WRITE_OVER_Z
                );
                break;

            case GIZMO_AXIS_Y:
                color = COLOR_BLUE;

                // Shaft
                draw_rectangle3d(
                    v3f(x - t, y + o,        z),
                    v3f(x + t, y + o,        z),
                    v3f(x - t, y + len - ah, z),
                    v3f(x + t, y + len - ah, z),
                    color, TRIANGLE_WRITE_OVER_Z
                );

                // Arrowhead
                draw_triangle3d(
                    v3f(x,      y + len + o,      z),
                    v3f(x - aw, y + len - ah + o, z),
                    v3f(x + aw, y + len - ah + o, z),
                    color, TRIANGLE_WRITE_OVER_Z
                );
                break;

            case GIZMO_AXIS_Z:
                color = COLOR_GREEN;

                // Shaft
                draw_rectangle3d(
                    v3f(x - t, y, z + o),
                    v3f(x + t, y, z + o),
                    v3f(x - t, y, z + len - ah),
                    v3f(x + t, y, z + len - ah),
                    color, TRIANGLE_WRITE_OVER_Z
                );

                // Arrowhead
                draw_triangle3d(
                    v3f(x,      y, z + len + o),
                    v3f(x - aw, y, z + len - ah + o),
                    v3f(x + aw, y, z + len - ah + o),
                    color, TRIANGLE_WRITE_OVER_Z
                );
                break;

                // Planar gizmos. 
            case GIZMO_AXIS_XZ: 
                draw_rectangle3d(
                    v3f(x + po + ps,       y, z  + ps),
                    v3f(x + po + ps,       y, z  + len - ps ),
                    v3f(x + po + len - ps, y, z  + ps),
                    v3f(x + po + len - ps, y, z  + len - ps),
                    COLOR_YELLOW, 0);
                break; 
            case GIZMO_AXIS_XY: 
                draw_rectangle3d(
                    v3f(x + po + ps,       y + ps,       z + len),
                    v3f(x + po + ps,       y + len - ps, z + len),
                    v3f(x + po + len - ps, y + ps,       z + len),
                    v3f(x + po + len - ps, y + len - ps, z + len),
                    COLOR_PURPLE, 0);
            break;
            case GIZMO_AXIS_YZ: 
                draw_rectangle3d(
                    v3f(x + len + po, y + ps,       z + ps  ),
                    v3f(x + len + po, y + ps,       z - ps + len ),
                    v3f(x + len + po, y - ps + len, z + ps ),
                    v3f(x + len + po, y - ps + len, z - ps + len ),
                    COLOR_BROWN,0 
                );
                break;
        }
    }
}

void gizmo_update(Gizmo *gizmo);

void gizmo_translation_modify(Gizmo *gizmo, Gizmo_Axis axis, V3f delta);
void gizmo_rotation_modify(Gizmo *gizmo, Gizmo_Axis axis, f32 angle);
Mat3 gizmo_get_rotation(Gizmo *gizmo);
