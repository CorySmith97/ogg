#include "gizmo.h"

void gizmo_draw(Gizmo *gizmo)
{
    for (s32 i = 0; i < GIZMO_AXIS_COUNT; i++) {

    }
}

void gizmo_translation_modify(Gizmo *gizmo, Gizmo_Axis axis, V3f delta);
void gizmo_rotation_modify(Gizmo *gizmo, Gizmo_Axis axis, f32 angle);
Mat3 gizmo_get_rotation(Gizmo *gizmo);
