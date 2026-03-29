#include "la.h"

/* V2f v2f(float x, float y)
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

int v2f_len(V2f v)
{
    return v2f_dot(v, v);
}

float v2f_dot(V2f v, V2f u)
{
    return v.x * u.x + v.y * u.y;
}

V2f v2f_scale(V2f v, float scalar)
{
    return (V2f){
        v.x * scalar,
        v.y * scalar,
    };
}

V2i v2i(int x, int y)
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

int v2i_len(V2i v)
{
    return v2i_dot(v, v);
}

float v2i_dot(V2i v, V2i u)
{
    return v.x * u.x + v.y * u.y;
}

V2i v2i_scale(V2i v, float scalar)
{
    return (V2i){
        v.x * scalar,
        v.y * scalar,
    };
}

float v2i_det(V2i v)
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
V2i v2i_rotate_by_v2i(V2i v, V2i c, float angle)
{
    return v2i(
        c.x + ((v.x - c.x) * cosf(angle)) - ((v.y - c.y) * sinf(angle)),
        c.y + ((v.x - c.x) * sinf(angle)) + ((v.y - c.y) * cosf(angle)));
}

V3f v3f(float x, float y, float z)
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

V3f v3f_scale(V3f v, float scalar)
{
    return v3f(v.x * scalar, v.y * scalar, v.z * scalar);
}

float v3f_len(V3f v)
{
    return v3f_dot(v, v);
}

float v3f_dot(V3f v, V3f u)
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

V3f v3f_rotate_y(V3f v, float angle)
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

V3f v3f_rotate_y_around_point(V3f v, V3f p, float angle)
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

V3f v3f_rotate_z_around_point(V3f v, V3f p, float angle)
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

V3f v3f_rotate_z(V3f v, float angle)
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
    float len = v3f_dot(v, v);
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

inline V4f v4f(float x, float y, float z, float w)
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

Mat3 rotation_y(float angle)
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

Mat3 rotation_z(float angle)
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

Mat3 rotation_x(float angle)
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

Mat4 perspective(float near, float far, float ar, float fov)
{
    return (Mat4){
        tanf(fov / 2) / ar,
        0,
        0,
        0,
        0,
        tanf(fov / 2),
        0,
        0,
        0,
        0,
        -((far + near) / (far - near)),
        -((2 * far * near) / (far - near)),
        0,
        0,
        -1,
        0,

    };
}
