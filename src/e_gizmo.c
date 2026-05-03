
Texture *plane_texture;

void gizmo_init(void)
{
    plane_texture = load_texture_from_file("data/textures/gizmo_plane.png", false);
}

static void draw_ring(V3f center, V3f axis_u, V3f axis_v,
                      f32 r, f32 half_t, Color color, s32 segments)
{
    // Normal to the ring plane — used for the axial quad so the ring is
    // visible even when viewed edge-on (e.g. X ring from a camera on the YZ plane).
    V3f ring_normal = v3f_cross(axis_u, axis_v);

    V3f cam_dir = v3f_normalize(v3f_sub(renderer.camera->position, center));
    f32 step = (2.0f * (f32)M_PI) / (f32)segments;

    for (s32 i = 0; i < segments; i++) {
        f32 a0   = i * step;
        f32 a1   = a0 + step;
        f32 amid = (a0 + a1) * 0.5f;

        V3f mid_dir = v3f_add(v3f_scale(axis_u, cosf(amid)),
                              v3f_scale(axis_v, sinf(amid)));
        if (v3f_dot(mid_dir, cam_dir) < 0.0f) continue;

        V3f d0 = v3f_add(v3f_scale(axis_u, cosf(a0)), v3f_scale(axis_v, sinf(a0)));
        V3f d1 = v3f_add(v3f_scale(axis_u, cosf(a1)), v3f_scale(axis_v, sinf(a1)));

        // Radial quad — wide in the ring's own plane
        V3f inner0 = v3f_add(center, v3f_scale(d0, r - half_t));
        V3f outer0 = v3f_add(center, v3f_scale(d0, r + half_t));
        V3f inner1 = v3f_add(center, v3f_scale(d1, r - half_t));
        V3f outer1 = v3f_add(center, v3f_scale(d1, r + half_t));
        draw_rectangle3d(inner0, outer0, inner1, outer1, color, TRIANGLE_WRITE_OVER_Z);

        // Axial quad — perpendicular to the ring plane so the ring is
        // always visible regardless of viewing angle
        V3f mid0 = v3f_add(center, v3f_scale(d0, r));
        V3f mid1 = v3f_add(center, v3f_scale(d1, r));
        V3f nm   = v3f_scale(ring_normal, half_t);
        draw_rectangle3d(v3f_sub(mid0, nm), v3f_add(mid0, nm),
                         v3f_sub(mid1, nm), v3f_add(mid1, nm),
                         color, TRIANGLE_WRITE_OVER_Z);
    }
}

void gizmo_draw(Gizmo *gizmo)
{
    float x = gizmo->position.x;
    float y = gizmo->position.y;
    float z = gizmo->position.z;

    if (gizmo->mode == GIZMO_MODE_ROTATE) {
        V3f pos = gizmo->position;

        // Highlight colors for active axis
        Color rx = (gizmo->active_axis == GIZMO_AXIS_X) ? (Color){255,140,140,255} : COLOR_RED;
        Color ry = (gizmo->active_axis == GIZMO_AXIS_Y) ? (Color){140,200,255,255} : COLOR_BLUE;
        Color rz = (gizmo->active_axis == GIZMO_AXIS_Z) ? (Color){140,255,140,255} : COLOR_GREEN;

        f32 ht_base   = 0.04f;
        f32 ht_active = 0.07f;

        // Slightly staggered radii to prevent Z-fighting
        draw_ring(pos, v3f(0,1,0), v3f(0,0,1), 1.00f,
                  gizmo->active_axis == GIZMO_AXIS_X ? ht_active : ht_base, rx, 32);
        draw_ring(pos, v3f(1,0,0), v3f(0,0,1), 1.05f,
                  gizmo->active_axis == GIZMO_AXIS_Y ? ht_active : ht_base, ry, 32);
        draw_ring(pos, v3f(1,0,0), v3f(0,1,0), 1.10f,
                  gizmo->active_axis == GIZMO_AXIS_Z ? ht_active : ht_base, rz, 32);
        return;
    }

    float len = 1.0f;
    float o   = 0.05f; // small offset along axis
    float po  = 0.3f; // small offset along axis
    float ps  = 0.10f; // small offset along axis
    float t   = 0.06f; // half-thickness
    float ah  = 0.12f; // arrowhead length
    float aw  = 0.06f; // arrowhead half-width

    for (s32 i = GIZMO_AXIS_COUNT - 1; i >= 0; i--) {
        Color color;

        switch (i) {
            case GIZMO_AXIS_X:
                color = COLOR_RED;

                //draw_gltf_model(get_gltf_model("arrow"), v3f(x+o, y, z), rotation_z(M_PI_2), false);

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

    if (gizmo->mode == GIZMO_MODE_ROTATE) {
        // Thin disc AABBs covering each ring's plane
        float r   = 1.15f;
        float eps = 0.07f;
        gizmo->aabbs[GIZMO_AXIS_X]  = (AABB){ v3f(x-eps, y-r, z-r), v3f(x+eps, y+r, z+r) };
        gizmo->aabbs[GIZMO_AXIS_Y]  = (AABB){ v3f(x-r, y-eps, z-r), v3f(x+r, y+eps, z+r) };
        gizmo->aabbs[GIZMO_AXIS_Z]  = (AABB){ v3f(x-r, y-r, z-eps), v3f(x+r, y+r, z+eps) };
        gizmo->aabbs[GIZMO_AXIS_XZ] = (AABB){0};
        gizmo->aabbs[GIZMO_AXIS_XY] = (AABB){0};
        gizmo->aabbs[GIZMO_AXIS_YZ] = (AABB){0};
        return;
    }

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
        default: break;
    }
    return ret;
}

void gizmo_rotation_modify(Gizmo *gizmo, Gizmo_Axis axis, f32 angle)
{
    switch (axis) {
        case GIZMO_AXIS_X: gizmo->axis_rotation[0] += angle; break;
        case GIZMO_AXIS_Y: gizmo->axis_rotation[1] += angle; break;
        case GIZMO_AXIS_Z: gizmo->axis_rotation[2] -= angle; break;
        default: break;
    }
}

Mat3 gizmo_get_rotation(Gizmo *gizmo)
{
    return mat3_mul(rotation_z(gizmo->axis_rotation[2]),
           mat3_mul(rotation_y(gizmo->axis_rotation[1]),
                    rotation_x(gizmo->axis_rotation[0])));
}
