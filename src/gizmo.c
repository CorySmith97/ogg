    #include "gizmo.h"

Texture *plane_texture;

void gizmo_init(void)
{
    plane_texture = load_texture_from_file("data/gizmo_plane.png", false);
}

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

    for (s32 i = GIZMO_AXIS_COUNT - 1; i >= 0; i--) {
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
                draw_texture3d(
                    plane_texture,
                    v3f(x + po + ps,       y, z  + ps),
                    v3f(x + po + ps,       y, z  + len - ps ),
                    v3f(x + po + len - ps, y, z  + ps),
                    v3f(x + po + len - ps, y, z  + len - ps),
                    COLOR_WHITE, TRIANGLE_WRITE_OVER_Z);
                break; 
            case GIZMO_AXIS_XY: 
                draw_texture3d(
                    plane_texture,
                    v3f(x + po + ps,       y + ps,       z + len),
                    v3f(x + po + ps,       y + len - ps, z + len),
                    v3f(x + po + len - ps, y + ps,       z + len),
                    v3f(x + po + len - ps, y + len - ps, z + len),
                    COLOR_WHITE, TRIANGLE_WRITE_OVER_Z);
            break;
            case GIZMO_AXIS_YZ: 
                draw_texture3d(
                    plane_texture,
                    v3f(x + len + po, y + ps,       z + ps  ),
                    v3f(x + len + po, y + ps,       z - ps + len ),
                    v3f(x + len + po, y - ps + len, z + ps ),
                    v3f(x + len + po, y - ps + len, z - ps + len ),
                    COLOR_WHITE, TRIANGLE_WRITE_OVER_Z);
                break;
        }
    }
}

void gizmo_update(Gizmo *gizmo)
{
    float x = gizmo->position.x;
    float y = gizmo->position.y;
    float z = gizmo->position.z;

    float len = 1.0f;
    float o   = 0.05f;
    float po  = 0.3f;
    float ps  = 0.10f;
    float t   = 0.06f;
    float ah  = 0.12f;
    float aw  = 0.06f;

    float eps = 0.02f; // small thickness for flat planes/picking


    // X axis = shaft rect + arrow tri
    {
        V3f shaft_pts[] = {
            v3f(x + o,        y - t,  z),
            v3f(x + len - ah, y - t,  z),
            v3f(x + o,        y + t,  z),
            v3f(x + len - ah, y + t,  z),
        };

        V3f arrow_pts[] = {
            v3f(x + len + o,      y,      z),
            v3f(x + len - ah + o, y - aw, z),
            v3f(x + len - ah + o, y + aw, z),
        };

        AABB shaft = aabb_from_points(shaft_pts, 4);
        AABB arrow = aabb_from_points(arrow_pts, 3);
        gizmo->aabbs[GIZMO_AXIS_X] = aabb_expand(aabb_merge(shaft, arrow), eps);
    }

    // Y axis = shaft rect + arrow tri
    {
        V3f shaft_pts[] = {
            v3f(x - t, y + o,        z),
            v3f(x + t, y + o,        z),
            v3f(x - t, y + len - ah, z),
            v3f(x + t, y + len - ah, z),
        };

        V3f arrow_pts[] = {
            v3f(x,      y + len + o,      z),
            v3f(x - aw, y + len - ah + o, z),
            v3f(x + aw, y + len - ah + o, z),
        };

        AABB shaft = aabb_from_points(shaft_pts, 4);
        AABB arrow = aabb_from_points(arrow_pts, 3);
        gizmo->aabbs[GIZMO_AXIS_Y] = aabb_expand(aabb_merge(shaft, arrow), eps);
    }

    // Z axis = shaft rect + arrow tri
    {
        V3f shaft_pts[] = {
            v3f(x - t, y, z + o),
            v3f(x + t, y, z + o),
            v3f(x - t, y, z + len - ah),
            v3f(x + t, y, z + len - ah),
        };

        V3f arrow_pts[] = {
            v3f(x,      y, z + len + o),
            v3f(x - aw, y, z + len - ah + o),
            v3f(x + aw, y, z + len - ah + o),
        };

        AABB shaft = aabb_from_points(shaft_pts, 4);
        AABB arrow = aabb_from_points(arrow_pts, 3);
        gizmo->aabbs[GIZMO_AXIS_Z] = aabb_expand(aabb_merge(shaft, arrow), eps);
    }

    // XZ plane
    {
        V3f pts[] = {
            v3f(x + po + ps,       y, z + ps),
            v3f(x + po + ps,       y, z + len - ps),
            v3f(x + po + len - ps, y, z + ps),
            v3f(x + po + len - ps, y, z + len - ps),
        };

        gizmo->aabbs[GIZMO_AXIS_XZ] = aabb_expand(aabb_from_points(pts, 4), eps);
    }

    // XY plane
    {
        V3f pts[] = {
            v3f(x + po + ps,       y + ps,       z + len),
            v3f(x + po + ps,       y + len - ps, z + len),
            v3f(x + po + len - ps, y + ps,       z + len),
            v3f(x + po + len - ps, y + len - ps, z + len),
        };

        gizmo->aabbs[GIZMO_AXIS_XY] = aabb_expand(aabb_from_points(pts, 4), eps);
    }

    // YZ plane
    {
        V3f pts[] = {
            v3f(x + len + po, y + ps,       z + ps),
            v3f(x + len + po, y + ps,       z + len - ps),
            v3f(x + len + po, y + len - ps, z + ps),
            v3f(x + len + po, y + len - ps, z + len - ps),
        };

        gizmo->aabbs[GIZMO_AXIS_YZ] = aabb_expand(aabb_from_points(pts, 4), eps);
    }
}

V3f gizmo_translation_modify(Gizmo *gizmo, Gizmo_Axis axis, V2f delta)
{
    V3f ret = {0};
    switch(axis) {
        case GIZMO_AXIS_X:
            ret.x += delta.x + delta.y;
            break;

        case GIZMO_AXIS_Y:
            ret.y -= delta.x + delta.y;
            break;

        case GIZMO_AXIS_Z:
            ret.z -= delta.x + delta.y;
            break;

            // Planar gizmos. 
        case GIZMO_AXIS_XZ: 
            ret.x += delta.x;
            ret.z -= delta.y;
            break; 
        case GIZMO_AXIS_XY: 
            ret.x += delta.x;
            ret.y -= delta.y;
            break;
        case GIZMO_AXIS_YZ: 
            ret.y -= delta.y;
            ret.z -= delta.x;
            break;
    }
    return ret;
}

void gizmo_rotation_modify(Gizmo *gizmo, Gizmo_Axis axis, f32 angle);
Mat3 gizmo_get_rotation(Gizmo *gizmo);
