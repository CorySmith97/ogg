#ifndef GIZMO
#define GIZMO

typedef enum {
    GIZMO_AXIS_X,
    GIZMO_AXIS_Y,
    GIZMO_AXIS_Z,
    GIZMO_AXIS_XZ,
    GIZMO_AXIS_XY,
    GIZMO_AXIS_YZ,
    GIZMO_AXIS_COUNT,
} Gizmo_Axis;

// There will be 3 separate modes of gizmos. 
// Translation
// Rotation
// Scaling
typedef struct {
    Gizmo_Axis  axis[GIZMO_AXIS_COUNT];
    f32         axis_rotation[GIZMO_AXIS_COUNT];
    AABB        aabbs[GIZMO_AXIS_COUNT];
    V3f         position;
    b32         attached;
} Gizmo;

void gizmo_draw(Gizmo *gizmo);
V3f  gizmo_translation_modify(Gizmo *gizmo, Gizmo_Axis axis, V2f delta);
void gizmo_rotation_modify(Gizmo *gizmo, Gizmo_Axis axis, f32 angle);
Mat3 gizmo_get_rotation(Gizmo *gizmo);

#endif // GIZMO
       
