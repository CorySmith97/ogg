#include "la.h"

/* V2f v2f(f32 x, f32 y)
{
    return (V2f){x, y};
} */

V2f v2f_add(V2f v, V2f u)
{
    return (V2f){
        v.x + u.x,
        v.y + u.y,
    };
}

V2f v2f_sub(V2f v, V2f u)
{
    return (V2f){
        v.x - u.x,
        v.y - u.y,
    };
}

f32 v2f_len(V2f v)
{
    return sqrtf(v2f_dot(v, v));
}

f32 v2f_dot(V2f v, V2f u)
{
    return v.x * u.x + v.y * u.y;
}

V2f v2f_scale(V2f v, f32 scalar)
{
    return (V2f){
        v.x * scalar,
        v.y * scalar,
    };
}

V2i v2i(s32 x, s32 y)
{
    return (V2i){x, y};
}

V2i v2i_add(V2i v, V2i u)
{
    return (V2i){
        v.x + u.x,
        v.y + u.y,
    };
}

V2i v2i_sub(V2i v, V2i u)
{
    return (V2i){
        v.x - u.x,
        v.y - u.y,
    };
}

f32 v2i_len(V2i v)
{
    return sqrtf(v2i_dot(v, v));
}

f32 v2i_dot(V2i v, V2i u)
{
    return v.x * u.x + v.y * u.y;
}

V2i v2i_scale(V2i v, f32 scalar)
{
    return (V2i){
        v.x * scalar,
        v.y * scalar,
    };
}

f32 v2i_det(V2i v)
{
    UNUSED(v);
    return 1.0;
}

V2i v2i_mul_mat2(V2i v, Mat2 m)
{
    return v2i(
        v.x * m.c[0] + v.x * m.c[2],
        v.y * m.c[1] + v.y * m.c[3]);
}

// This rotation is lossy
V2i v2i_rotate_by_v2i(V2i v, V2i c, f32 angle)
{
    return v2i(
        c.x + ((v.x - c.x) * cosf(angle)) - ((v.y - c.y) * sinf(angle)),
        c.y + ((v.x - c.x) * sinf(angle)) + ((v.y - c.y) * cosf(angle)));
}

V3f v3f(f32 x, f32 y, f32 z)
{
    return (V3f){
        x,
        y,
        z,
    };
}

V3f v3f_add(V3f v, V3f u)
{
    return (V3f){
        v.x + u.x,
        v.y + u.y,
        v.z + u.z,
    };
}

V3f v3f_sub(V3f v, V3f u)
{
    return (V3f){
        v.x - u.x,
        v.y - u.y,
        v.z - u.z,
    };
}

V3f v3f_scale(V3f v, f32 scalar)
{
    return v3f(v.x * scalar, v.y * scalar, v.z * scalar);
}

f32 v3f_len(V3f v)
{
    return sqrtf(v3f_dot(v, v));
}

f32 v3f_dot(V3f v, V3f u)
{
    return (v.x * u.x) + (v.y * u.y) + (v.z * u.z);
}

V3f v3f_mul(V3f v, V3f u)
{
    return v3f(v.x * u.x, v.y * u.y, v.z * u.z);
}

V3f v3f_mul_mat3(V3f v, Mat3 m)
{
    return (V3f){
        v.x * m.c[0] + v.y * m.c[1] + v.z * m.c[2],
        v.x * m.c[3] + v.y * m.c[4] + v.z * m.c[5],
        v.x * m.c[6] + v.y * m.c[7] + v.z * m.c[8],
    };
}

V3f v3f_rotate_y(V3f v, f32 angle)
{
    Mat3 rotation = {
        cosf(angle),
        0,
        sinf(angle),
        0,
        1,
        0,
        -sinf(angle),
        0,
        cosf(angle),
    };

    return v3f_mul_mat3(v, rotation);
}

V3f v3f_rotate_y_around_point(V3f v, V3f p, f32 angle)
{
    Mat3 rotation = {
        cosf(angle),
        0,
        sinf(angle),
        0,
        1,
        0,
        -sinf(angle),
        0,
        cosf(angle),
    };

    return v3f_add(p, v3f_mul_mat3(v3f_sub(v, p), rotation));
}

V3f v3f_rotate_z_around_point(V3f v, V3f p, f32 angle)
{
    Mat3 rotation = {
        cosf(angle),
        sinf(angle),
        0,
        -sinf(angle),
        cosf(angle),
        0,
        0,
        0,
        1,
    };

    return v3f_add(p, v3f_mul_mat3(v3f_sub(v, p), rotation));
}

V3f v3f_rotate_z(V3f v, f32 angle)
{
    Mat3 rotation = {
        cosf(angle),
        sinf(angle),
        0,
        -sinf(angle),
        cosf(angle),
        0,
        0,
        0,
        1,
    };

    return v3f_mul_mat3(v, rotation);
}

V3f v3f_normalize(V3f v)
{
    f32 len = sqrtf(v3f_dot(v, v));
    return v3f_scale(v, 1 / len);
}

V3f v3f_cross(V3f v, V3f u)
{
    return (V3f){
        v.y * u.z - v.z * u.y,
        v.z * u.x - v.x * u.z,
        v.x * u.y - v.y * u.x,
    };
}

inline V4f v4f(f32 x, f32 y, f32 z, f32 w)
{
    return (V4f){
        x,
        y,
        z,
        w,
    };
}

V4f v4f_mul_mat4(V4f v, Mat4 m)
{
    return (V4f){
        v.x * m.c[0] + v.y * m.c[1] + v.z * m.c[2] + v.w * m.c[3],
        v.x * m.c[4] + v.y * m.c[5] + v.z * m.c[6] + v.w * m.c[7],
        v.x * m.c[8] + v.y * m.c[9] + v.z * m.c[10] + v.w * m.c[11],
        v.x * m.c[12] + v.y * m.c[13] + v.z * m.c[14] + v.w * m.c[15],
    };
}

Mat3 rotation_y(f32 angle)
{
    return (Mat3){
        cosf(angle),
        0,
        sinf(angle),
        0,
        1,
        0,
        -sinf(angle),
        0,
        cosf(angle),
    };
}

Mat3 rotation_z(f32 angle)
{
    return (Mat3){
        cosf(angle),
        -sinf(angle),
        0,
        sinf(angle),
        cosf(angle),
        0,
        0,
        0,
        1,
    };
}

Mat3 rotation_x(f32 angle)
{
    return (Mat3){
        1,
        0,
        0,
        0,
        cosf(angle),
        -sinf(angle),
        0,
        sinf(angle),
        cosf(angle),
    };
}


Mat3 mat3_scale(f32 scalar)
{
    return (Mat3){
        scalar,
        0,
        0,
        0,
        scalar,
        0,
        0,
        0,
        scalar,
    };
}

Mat3 mat3_mul(Mat3 m1, Mat3 m2)
{
    return (Mat3){
        // Row 0 of m1 · Columns of m2
        m1.c[0] * m2.c[0] + m1.c[1] * m2.c[3] + m1.c[2] * m2.c[6],
        m1.c[0] * m2.c[1] + m1.c[1] * m2.c[4] + m1.c[2] * m2.c[7],
        m1.c[0] * m2.c[2] + m1.c[1] * m2.c[5] + m1.c[2] * m2.c[8],

        // Row 1 of m1 · Columns of m2
        m1.c[3] * m2.c[0] + m1.c[4] * m2.c[3] + m1.c[5] * m2.c[6],
        m1.c[3] * m2.c[1] + m1.c[4] * m2.c[4] + m1.c[5] * m2.c[7],
        m1.c[3] * m2.c[2] + m1.c[4] * m2.c[5] + m1.c[5] * m2.c[8],

        // Row 2 of m1 · Columns of m2
        m1.c[6] * m2.c[0] + m1.c[7] * m2.c[3] + m1.c[8] * m2.c[6],
        m1.c[6] * m2.c[1] + m1.c[7] * m2.c[4] + m1.c[8] * m2.c[7],
        m1.c[6] * m2.c[2] + m1.c[7] * m2.c[5] + m1.c[8] * m2.c[8],
    };
}

Mat3 mat3_add(Mat3 m1, Mat3 m2)
{
    return (Mat3){
        m1.c[0] + m2.c[0],
        m1.c[1] + m2.c[1],
        m1.c[2] + m2.c[2],
        m1.c[3] + m2.c[3],
        m1.c[4] + m2.c[4],
        m1.c[5] + m2.c[5],
        m1.c[6] + m2.c[6],
        m1.c[7] + m2.c[7],
        m1.c[8] + m2.c[8],
    };
}

Mat3 mat3_identity(void)
{
    return (Mat3){
        1,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        1,
    };
}

Mat4 mat4_identity(void)
{
    return (Mat4){
        1,
        0,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
        1,
    };
}

void camera_update(Camera *cam)
{
    UNUSED(cam);
}

Mat4 camera_matrix(Camera cam)
{
    V3f f = v3f_normalize(cam.front);
    V3f r = v3f_normalize(v3f_cross(cam.up, f));
    V3f u = v3f_cross(f, r);

    return (Mat4){
        r.x, r.y, r.z, -v3f_dot(r, cam.position),
        u.x, u.y, u.z, -v3f_dot(u, cam.position),
        f.x, f.y, f.z, -v3f_dot(f, cam.position),
        0, 0, 0, 1,
    };
}

Mat4 mat4_mul(Mat4 m1, Mat4 m2)
{
    return (Mat4){
        // Row 0 of m1 · Columns of m2
        m1.c[0] * m2.c[0] + m1.c[1] * m2.c[4] + m1.c[2] * m2.c[8] + m1.c[3] * m2.c[12],
        m1.c[0] * m2.c[1] + m1.c[1] * m2.c[5] + m1.c[2] * m2.c[9] + m1.c[3] * m2.c[13],
        m1.c[0] * m2.c[2] + m1.c[1] * m2.c[6] + m1.c[2] * m2.c[10] + m1.c[3] * m2.c[14],
        m1.c[0] * m2.c[3] + m1.c[1] * m2.c[7] + m1.c[2] * m2.c[11] + m1.c[3] * m2.c[15],

        // Row 1 of m1 · Columns of m2
        m1.c[4] * m2.c[0] + m1.c[5] * m2.c[4] + m1.c[6] * m2.c[8] + m1.c[7] * m2.c[12],
        m1.c[4] * m2.c[1] + m1.c[5] * m2.c[5] + m1.c[6] * m2.c[9] + m1.c[7] * m2.c[13],
        m1.c[4] * m2.c[2] + m1.c[5] * m2.c[6] + m1.c[6] * m2.c[10] + m1.c[7] * m2.c[14],
        m1.c[4] * m2.c[3] + m1.c[5] * m2.c[7] + m1.c[6] * m2.c[11] + m1.c[7] * m2.c[15],

        // Row 2 of m1 · Columns of m2
        m1.c[8] * m2.c[0] + m1.c[9] * m2.c[4] + m1.c[10] * m2.c[8] + m1.c[11] * m2.c[12],
        m1.c[8] * m2.c[1] + m1.c[9] * m2.c[5] + m1.c[10] * m2.c[9] + m1.c[11] * m2.c[13],
        m1.c[8] * m2.c[2] + m1.c[9] * m2.c[6] + m1.c[10] * m2.c[10] + m1.c[11] * m2.c[14],
        m1.c[8] * m2.c[3] + m1.c[9] * m2.c[7] + m1.c[10] * m2.c[11] + m1.c[11] * m2.c[15],

        m1.c[12] * m2.c[0] + m1.c[13] * m2.c[4] + m1.c[14] * m2.c[8] + m1.c[15] * m2.c[12],
        m1.c[12] * m2.c[1] + m1.c[13] * m2.c[5] + m1.c[14] * m2.c[9] + m1.c[15] * m2.c[13],
        m1.c[12] * m2.c[2] + m1.c[13] * m2.c[6] + m1.c[14] * m2.c[10] + m1.c[15] * m2.c[14],
        m1.c[12] * m2.c[3] + m1.c[13] * m2.c[7] + m1.c[14] * m2.c[11] + m1.c[15] * m2.c[15],
    };
}

V3f v3f_translate_by_mat4(V3f v, Mat4 m)
{
    V4f v1 = v4f(v.x, v.y, v.z, 1);

    V4f translated = v4f_mul_mat4(v1, m);

    return v3f(translated.x, translated.y, translated.z);
}

Mat4 perspective(f32 near, f32 far, f32 ar, f32 fov)
{
    f32 f = 1.0f / tanf(fov / 2.0f);
    return (Mat4){
        f/ar, 0,  0,   0,
        0,    f,  0,   0,
        0,    0, -(far+near)/(far-near), -1,
        0,    0, -(2*far*near)/(far-near), 0,
    };
}

Mat4 look_at(V3f position, V3f target, V3f up)
{
    V3f f = v3f_normalize(v3f_sub(target, position));
    V3f r = v3f_normalize(v3f_cross(up, f));
    V3f u = v3f_cross(f, r);

    return (Mat4){
        r.x, r.y, r.z, -v3f_dot(r, position),
        u.x, u.y, u.z, -v3f_dot(u, position),
        f.x, f.y, f.z, -v3f_dot(f, position),
        0, 0, 0, 1,
    };
}

V3f v3f_unproject(V3f source, Mat4 projection, Mat4 view)
{
    // view * projection, matching your mat4_mul convention
    Mat4 view_proj = mat4_mul(view, projection);

    // Cache elements for inversion
    f32 a00 = view_proj.c[0],  a01 = view_proj.c[1],  a02 = view_proj.c[2],  a03 = view_proj.c[3];
    f32 a10 = view_proj.c[4],  a11 = view_proj.c[5],  a12 = view_proj.c[6],  a13 = view_proj.c[7];
    f32 a20 = view_proj.c[8],  a21 = view_proj.c[9],  a22 = view_proj.c[10], a23 = view_proj.c[11];
    f32 a30 = view_proj.c[12], a31 = view_proj.c[13], a32 = view_proj.c[14], a33 = view_proj.c[15];

    f32 b00 = a00*a11 - a01*a10;
    f32 b01 = a00*a12 - a02*a10;
    f32 b02 = a00*a13 - a03*a10;
    f32 b03 = a01*a12 - a02*a11;
    f32 b04 = a01*a13 - a03*a11;
    f32 b05 = a02*a13 - a03*a12;
    f32 b06 = a20*a31 - a21*a30;
    f32 b07 = a20*a32 - a22*a30;
    f32 b08 = a20*a33 - a23*a30;
    f32 b09 = a21*a32 - a22*a31;
    f32 b10 = a21*a33 - a23*a31;
    f32 b11 = a22*a33 - a23*a32;

    f32 det = b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06;

    // Optional safety check
    if (fabsf(det) < 1e-8f) {
        return v3f(0.0f, 0.0f, 0.0f);
    }

    f32 inv_det = 1.0f / det;

    Mat4 inv = {
        (a11*b11 - a12*b10 + a13*b09) * inv_det,
        (-a01*b11 + a02*b10 - a03*b09) * inv_det,
        (a31*b05 - a32*b04 + a33*b03) * inv_det,
        (-a21*b05 + a22*b04 - a23*b03) * inv_det,

        (-a10*b11 + a12*b08 - a13*b07) * inv_det,
        (a00*b11 - a02*b08 + a03*b07) * inv_det,
        (-a30*b05 + a32*b02 - a33*b01) * inv_det,
        (a20*b05 - a22*b02 + a23*b01) * inv_det,

        (a10*b10 - a11*b08 + a13*b06) * inv_det,
        (-a00*b10 + a01*b08 - a03*b06) * inv_det,
        (a30*b04 - a31*b02 + a33*b00) * inv_det,
        (-a20*b04 + a21*b02 - a23*b00) * inv_det,

        (-a10*b09 + a11*b07 - a12*b06) * inv_det,
        (a00*b09 - a01*b07 + a02*b06) * inv_det,
        (-a30*b03 + a31*b01 - a32*b00) * inv_det,
        (a20*b03 - a21*b01 + a22*b00) * inv_det
    };

    // source is assumed to already be in NDC:
    // x, y, z in [-1, 1]
    V4f p = v4f(source.x, source.y, source.z, 1.0f);
    V4f t = v4f_mul_mat4(p, inv);

    if (fabsf(t.w) < 1e-8f) {
        return v3f(0.0f, 0.0f, 0.0f);
    }

    return v3f(t.x / t.w, t.y / t.w, t.z / t.w);
}
