#ifndef GIZMO
#define GIZMO

typedef enum {
    GIZMO_AXIS_X,
    GIZMO_AXIS_Y,
    GIZMO_AXIS_Z,
    GIZMO_AXIS_Count,
} Gizmo_Axis;

typedef struct {
    Gizmo_Axis axis;
} Gizmo;

#endif // GIZMO
