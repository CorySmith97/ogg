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

