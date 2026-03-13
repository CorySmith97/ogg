#ifndef LA_H
#define LA_H

typedef union V2f V2f;
union V2f 
{
    struct {float x, y;};
    struct {float w, h;};
    float c[2];
};

typedef union V2i V2i;
union V2i 
{
    struct {int x, y;};
    struct {int w, h;};
    int c[2];
};

typedef union V3f V3f;
union V3f 
{
    struct {float x, y, z;};
    float c[3];
};

typedef union V3i V3i;
union V3i 
{
    struct {int x, y, z;};
    int c[3];
};

typedef union Mat4 Mat4;
union Mat4
{
    float c[16];
};

// The camera is just primarily a mental model. The world actually moves
// around the camera as opposed to the camera.
typedef struct Camera 
{
    V3f position;
    V3f up;
    V3f target;
    float fovy;
} Camera;

void camera_update(Camera *cam);
Mat4 camera_matrix(Camera cam);


V2f   v2f(float x, float y);
V2f   v2f_add(V2f v, V2f u);
V2f   v2f_sub(V2f v, V2f u);
V2f   v2f_scale(V2f v, float scalar);
int   v2f_len(V2f v);
float v2f_dot(V2f v, V2f u);

V2i   v2i(int x, int y);
V2i   v2i_add(V2i v, V2i u);
V2i   v2i_sub(V2i v, V2i u);
V2i   v2i_scale(V2i v, float scalar);
int   v2i_len(V2i v);
float v2i_dot(V2i v, V2i u);
float v2i_det(V2i v);

V3f   v3f(float x, float y, float z);
V3f   v3f_add(V3f v, V3f u);
V3f   v3f_sub(V3f v, V3f u);
V3f   v3f_scale(V3f v, float scalar);
float v3f_len(V3f v);
float v3f_dot(V3f v, V3f u);

V3i   v3i(int x, int y, int z);
V3i   v3i_add(V3i v, V3i u);
V3i   v3i_sub(V3i v, V3i u);
V3i   v3i_scale(V3i v, float scalar);
float v3i_len(V3i v);
float v3i_dot(V3i v, V3i u);

#endif  // LA_H
