#include "la.h"

V2f v2f(float x, float y)
{
    return (V2f){x, y};
}

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
    return (V3f) {
        x, y, z,
    };
}

V3f   
v3f_add(V3f v, V3f u)
{
    return (V3f){
        v.x + u.x,
        v.y + u.y,
        v.z + u.z,
    };
}

V3f   
v3f_sub(V3f v, V3f u)
{
    return (V3f){
        v.x - u.x,
        v.y - u.y,
        v.z - u.z,
    };
}

V3f   
v3f_scale(V3f v, float scalar)
{
    return v3f(v.x * scalar, v.y * scalar, v.z * scalar);
}

float 
v3f_len(V3f v)
{
    return v3f_dot(v, v);
}

float 
v3f_dot(V3f v, V3f u)
{
    return (v.x * u.x) + (v.y * u.y) + (v.z * u.z);
}

V3f   
v3f_mul_mat3(V3f v, Mat3 m)
{
    return (V3f){
        v.x * m.c[0] + v.y * m.c[1] + v.z * m.c[2],
        v.x * m.c[3] + v.y * m.c[4] + v.z * m.c[5],
        v.x * m.c[6] + v.y * m.c[7] + v.z * m.c[8],
    };
}

V3f   
v3f_rotate_y(V3f v, float angle)
{
    Mat3 rotation = {
        cosf(angle), 0, sinf(angle),
        0,           1,           0,
        -sinf(angle),0, cosf(angle),
    };

    return v3f_mul_mat3(v, rotation);
}

V3f   
v3f_rotate_y_around_point(V3f v, V3f p, float angle)
{
    Mat3 rotation = {
        cosf(angle), 0, sinf(angle),
        0,           1,           0,
        -sinf(angle),0, cosf(angle),
    };

    return v3f_add(p, v3f_mul_mat3(v3f_sub(v,p), rotation));
}

V3f   
v3f_rotate_z_around_point(V3f v, V3f p, float angle)
{
    Mat3 rotation = {
        cosf(angle),  sinf(angle),0, 
        -sinf(angle), cosf(angle),0, 
        0,            0,          1, 
    };

    return v3f_add(p, v3f_mul_mat3(v3f_sub(v,p), rotation));
}

V3f   
v3f_rotate_z(V3f v, float angle)
{
    Mat3 rotation = {
        cosf(angle), sinf(angle), 0,
        -sinf(angle),cosf(angle),0,
        0,           0,           1,
    };

    return v3f_mul_mat3(v, rotation);
}

Mat3 rotation_y(float angle)
{
    return  (Mat3){
        cosf(angle), 0, sinf(angle),
        0,           1,           0,
        -sinf(angle),0, cosf(angle),
    };
}

Mat3 rotation_z(float angle)
{
    return  (Mat3){
        cosf(angle), -sinf(angle),0, 
        sinf(angle), cosf(angle),0, 
        0,            0,          1, 
    };
}

Mat3 rotation_x(float angle)
{
    return  (Mat3){
        1, 0,            0,           
        0, cosf(angle),  -sinf(angle), 
        0, sinf(angle), cosf(angle), 
    };
}

Mat3 mat3_mul(Mat3 m1, Mat3 m2)
{
    return (Mat3){
        // Row 0 of m1 · Columns of m2
        m1.c[0]*m2.c[0] + m1.c[1]*m2.c[3] + m1.c[2]*m2.c[6],
        m1.c[0]*m2.c[1] + m1.c[1]*m2.c[4] + m1.c[2]*m2.c[7],
        m1.c[0]*m2.c[2] + m1.c[1]*m2.c[5] + m1.c[2]*m2.c[8],

        // Row 1 of m1 · Columns of m2
        m1.c[3]*m2.c[0] + m1.c[4]*m2.c[3] + m1.c[5]*m2.c[6],
        m1.c[3]*m2.c[1] + m1.c[4]*m2.c[4] + m1.c[5]*m2.c[7],
        m1.c[3]*m2.c[2] + m1.c[4]*m2.c[5] + m1.c[5]*m2.c[8],

        // Row 2 of m1 · Columns of m2
        m1.c[6]*m2.c[0] + m1.c[7]*m2.c[3] + m1.c[8]*m2.c[6],
        m1.c[6]*m2.c[1] + m1.c[7]*m2.c[4] + m1.c[8]*m2.c[7],
        m1.c[6]*m2.c[2] + m1.c[7]*m2.c[5] + m1.c[8]*m2.c[8],
    };
}
Mat3 mat3_add(Mat3 m1, Mat3 m2)
{
    return (Mat3){
        m1.c[0] + m2.c[0], m1.c[1] + m2.c[1], m1.c[2] + m2.c[2],
        m1.c[3] + m2.c[3], m1.c[4] + m2.c[4], m1.c[5] + m2.c[5],
        m1.c[6] + m2.c[6], m1.c[7] + m2.c[7], m1.c[8] + m2.c[8],
    };
}
