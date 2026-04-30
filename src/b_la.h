#ifndef LA_H
#define LA_H

typedef union V2f V2f;
union V2f {
    struct {
        f32 x, y;
    };
    struct {
        f32 w, h;
    };
    f32 c[2];
};

typedef union V2i V2i;
union V2i {
    struct {
        s32 x, y;
    };
    struct {
        s32 w, h;
    };
    s32 c[2];
};

typedef union V3f V3f;
union V3f {
    struct {
        f32 x, y, z;
    };
    f32 c[3];
};

typedef union V3i V3i;
union V3i {
    struct {
        s32 x, y, z;
    };
    s32 c[3];
};

typedef union V4f V4f;
union V4f {
    struct {
        f32 x, y, z, w;
    };
    f32 c[4];
};

typedef union Mat2 Mat2;
union Mat2 {
    f32 c[4];
};

typedef union Mat3 Mat3;
union Mat3 {
    f32 c[9];
};

typedef union Mat4 Mat4;
union Mat4 {
    f32 c[16];
    f32 m[4][4];
};

typedef struct {
    f32 x, y, z, w;
} Quat;

typedef struct Camera {
    V3f target;
    V3f position;
    V3f up;
    V3f front;
    f32 fovy;
    f32 distance;
    f32 pitch;
    f32 yaw;
} Camera;

void camera_update(Camera *cam);
Mat4 camera_matrix(Camera *cam);

// TODO i think the contructors should be macros?

#define v2f(_x, _y) (V2f){(_x), (_y)}
//V2f   v2f(f32 x, f32 y);
V2f v2f_add(V2f v, V2f u);
V2f v2f_sub(V2f v, V2f u);
V2f v2f_scale(V2f v, f32 scalar);
f32 v2f_len(V2f v);
f32 v2f_dot(V2f v, V2f u);

V2i v2i(s32 x, s32 y);
V2i v2i_add(V2i v, V2i u);
V2i v2i_sub(V2i v, V2i u);
V2i v2i_scale(V2i v, f32 scalar);
f32 v2i_len(V2i v);
f32 v2i_dot(V2i v, V2i u);
f32 v2i_det(V2i v);
V2i v2i_rotate(V2i v, f32 angle);

V3f v3f(f32 x, f32 y, f32 z);
V3f v3f_add(V3f v, V3f u);
V3f v3f_sub(V3f v, V3f u);
V3f v3f_scale(V3f v, f32 scalar);
f32 v3f_len(V3f v);
f32 v3f_dot(V3f v, V3f u);
V3f v3f_mul_mat3(V3f v, Mat3 m);
V3f v3f_rotate_y(V3f v, f32 angle);
V3f v3f_rotate_z(V3f v, f32 angle);
V3f v3f_rotate_y_around_pos32(V3f v, V3f p, f32 angle);
V3f v3f_rotate_z_around_pos32(V3f v, V3f p, f32 angle);
V3f v3f_normalize(V3f v);
V3f v3f_translate_by_mat4(V3f v, Mat4 m);
static inline V3f v3f_min(V3f a, V3f b);
static inline V3f v3f_max(V3f a, V3f b);
b32 v3f_equal(V3f v, V3f u);

V3i v3i(s32 x, s32 y, s32 z);
V3i v3i_add(V3i v, V3i u);
V3i v3i_sub(V3i v, V3i u);
V3i v3i_scale(V3i v, f32 scalar);
f32 v3i_len(V3i v);
f32 v3i_dot(V3i v, V3i u);

V4f v4f(f32 x, f32 y, f32 z, f32 w); 
V4f v4f_mul_mat4(V4f v, Mat4 m);

Mat3 mat3_identity(void);
Mat3 mat3_scale(f32 scalar);
Mat3 mat3_mul(Mat3 m1, Mat3 m2);
Mat3 rotation_x(f32 angle);
Mat3 rotation_y(f32 angle);
Mat3 rotation_z(f32 angle);
Mat3 mat3_add(Mat3 m1, Mat3 m2);

Mat4 mat4_identity(void);
Mat4 mat4_mul(Mat4 m1, Mat4 m2);
Mat4 perspective(f32 near, f32 far, f32 ar, f32 fov);
Mat4 look_at(V3f position, V3f target, V3f up);

Quat quat(f32 x, f32 y, f32 z, f32 w);
Quat quat_identity(void);
Quat quat_normalise(Quat q);
Quat quat_mul(Quat a, Quat b);
V3f  quat_rotate_v3f(Quat q, V3f v);
// AI-GENERATED: convert a row-major Mat3 to a unit quaternion (Shepperd's method).
// Handles the four degenerate cases near 180-degree rotations.
Quat mat3_to_quat(Mat3 m);

Mat4 mat4_from_quat(Quat q);
Mat4 mat4_inverse(Mat4 m);

#endif // LA_H
