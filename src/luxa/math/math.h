#pragma once

#include <luxa/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <float.h>

#define LX_SEC2MS               1000.0f
#define	LX_MS2SEC               0.001f
#define	LX_1BYPI                0.318309886f
#define	LX_PI                   3.141592654f
#define	LX_2PI                  6.283185307f
#define	LX_PI_OVER_2            (0.5f * LX_PI)
#define	LX_180_OVER_PI          (180.0f / LX_PI)
#define	LX_PI_OVER_180          (LX_PI / 180.0f)
#define	LX_FLOAT_EPSILON        1.192092896e-07F
#define LX_MAT_INVERSE_EPSILON  1e-14
#define	LX_MAT_EPSILON          1e-6

typedef struct lx_vec2 {
	float x;
	float y;
} lx_vec2_t;

typedef struct lx_vec3 {
	float x;
	float y;
	float z;
} lx_vec3_t;

LX_ALIGN16 typedef struct lx_vec4 {
	float x;
	float y;
	float z;
	float w;
} lx_vec4_t;

LX_ALIGN16 typedef struct lx_quat {
    float x;
    float y;
    float z;
    float w;
} lx_quat_t;

LX_ALIGN16 typedef struct lx_mat4 {
    union {
        struct {
            float m11, m12, m13, m14;
            float m21, m22, m23, m24;
            float m31, m32, m33, m34;
            float m41, m42, m43, m44;
        };
        float m[16];
    };
} lx_mat4_t;

typedef struct lx_extent2 {
	uint32_t width;
	uint32_t height;
} lx_extent2_t;

#define lx_sqrtf sqrtf

#define lx_radians(degrees) (degrees * LX_PI_OVER_180)

#define lx_degrees(radians) (radians * LX_180_OVER_PI)

/*
 * 2-D Vector.
 */
static LX_INLINE void lx_vec2_zero(lx_vec2_t *v)
{
    *v = (lx_vec2_t) { 0 };
}

static LX_INLINE float lx_vec2_at(const lx_vec2_t *v, size_t i)
{
    return ((const float *)v)[i];
}

static LX_INLINE void lx_vec2_add(const lx_vec2_t *a, const lx_vec2_t *b, lx_vec2_t *out)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
}

static LX_INLINE void lx_vec2_sub(const lx_vec2_t *a, const lx_vec2_t *b, lx_vec2_t *out)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
}

static LX_INLINE void lx_vec2_mul(const lx_vec2_t *a, const lx_vec2_t *b, lx_vec2_t *out)
{
    out->x = a->x * b->x;
    out->y = a->y * b->y;
}

static LX_INLINE void lx_vec2_div(const lx_vec2_t *a, const lx_vec2_t *b, lx_vec2_t *out)
{
    out->x = a->x / b->x;
    out->y = a->y / b->y;
}

static LX_INLINE void lx_vec2_scale(const lx_vec2_t *a, float s, lx_vec2_t *out)
{
    out->x = a->x * s;
    out->y = a->y * s;
}

static LX_INLINE float lx_vec2_dot(const lx_vec2_t *a, const lx_vec2_t *b)
{
    return a->x * b->x + a->y * b->y;
}

static LX_INLINE void lx_vec2_cross(const lx_vec2_t *a, const lx_vec2_t *b, lx_vec2_t *out)
{
    out->x = a->x * b->y - a->y * b->x;
    out->y = a->x * b->y - a->y * b->x;
}

static LX_INLINE float lx_vec2_squared_length(const lx_vec2_t *v)
{
    return lx_vec2_dot(v, v);
}

static LX_INLINE float lx_vec2_length(const lx_vec2_t *v)
{
    return lx_sqrtf(lx_vec2_squared_length(v));
}

static LX_INLINE void lx_vec2_normalize(const lx_vec2_t *v, lx_vec2_t *out)
{
    float length = lx_vec2_squared_length(v);
    if (length > 0.0f) {
        float s = 1.0f / lx_sqrtf(length);
        out->x = v->x * s;
        out->y = v->y * s;
    }
}

static LX_INLINE float lx_vec2_squared_distance(const lx_vec2_t *a, const lx_vec2_t *b)
{
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    return dx * dx + dy * dy;
}

static LX_INLINE float lx_vec2_distance(const lx_vec2_t *a, const lx_vec2_t *b)
{
    return lx_sqrtf(lx_vec2_squared_distance(a, b));
}

/*
 * 3-D Vector.
 */
static LX_INLINE void lx_vec3_zero(lx_vec3_t *v)
{
    *v = (lx_vec3_t) { 0 };
}

static LX_INLINE float lx_vec3_at(const lx_vec3_t *v, size_t i)
{
    return ((const float *)v)[i];
}

static LX_INLINE void lx_vec3_add(const lx_vec3_t *a, const lx_vec3_t *b, lx_vec3_t *out)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
}

LX_INLINE lx_vec3_t *lx_vec3_sub(const lx_vec3_t *a, const lx_vec3_t *b, lx_vec3_t *out)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
    return out;
}

static LX_INLINE void lx_vec3_mul(const lx_vec3_t *a, const lx_vec3_t *b, lx_vec3_t *out)
{
    out->x = a->x * b->x;
    out->y = a->y * b->y;
    out->z = a->z * b->z;
}

static LX_INLINE void lx_vec3_div(const lx_vec3_t *a, const lx_vec3_t *b, lx_vec3_t *out)
{
    out->x = a->x / b->x;
    out->y = a->y / b->y;
    out->z = a->z / b->z;
}

static LX_INLINE void lx_vec3_scale(const lx_vec3_t *a, float s, lx_vec3_t *out)
{
    out->x = a->x * s;
    out->y = a->y * s;
    out->z = a->z * s;
}

static LX_INLINE float lx_vec3_dot(const lx_vec3_t *a, const lx_vec3_t *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

static LX_INLINE lx_vec3_t *lx_vec3_cross(const lx_vec3_t *a, const lx_vec3_t *b, lx_vec3_t *out)
{
    out->x = a->y * b->z - a->z * b->y;
    out->y = a->z * b->x - a->x * b->z;
    out->z = a->x * b->y - a->y * b->x;
    return out;
}

static LX_INLINE float lx_vec3_squared_length(const lx_vec3_t *v)
{
    return lx_vec3_dot(v, v);
}

static LX_INLINE float lx_vec3_length(const lx_vec3_t *v)
{
    return lx_sqrtf(lx_vec3_squared_length(v));
}

static LX_INLINE void lx_vec3_normalize(lx_vec3_t *v)
{
    float length = lx_vec3_squared_length(v);
    if (length > 0.0f) {
        float s = 1.0f / lx_sqrtf(length);
        v->x *= s;
        v->y *= s;
        v->z *= s;
    }
}

static LX_INLINE float lx_vec3_squared_distance(const lx_vec3_t *a, const lx_vec3_t *b)
{
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    float dz = a->z - b->z;
    return dx * dx + dy * dy + dz * dz;
}

static LX_INLINE float lx_vec3_distance(const lx_vec3_t *a, const lx_vec3_t *b)
{
    return lx_sqrtf(lx_vec3_squared_distance(a, b));
}

/*
 * 4-D Vector.
 */
static LX_INLINE void lx_vec4_zero(lx_vec4_t *v)
{
    *v = (lx_vec4_t) { 0 };
}

static LX_INLINE float lx_vec4_at(const lx_vec4_t *v, size_t i)
{
    return ((const float *)v)[i];
}

static LX_INLINE void lx_vec4_add(const lx_vec4_t *a, const lx_vec4_t *b, lx_vec4_t *out)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
    out->w = a->w + b->w;
}

static LX_INLINE void lx_vec4_sub(const lx_vec4_t *a, const lx_vec4_t *b, lx_vec4_t *out)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
    out->w = a->w - b->w;
}

static LX_INLINE void lx_vec4_mul(const lx_vec4_t *a, const lx_vec4_t *b, lx_vec4_t *out)
{
    out->x = a->x * b->x;
    out->y = a->y * b->y;
    out->z = a->z * b->z;
    out->w = a->w * b->w;
}

static LX_INLINE void lx_vec4_div(const lx_vec4_t *a, const lx_vec4_t *b, lx_vec4_t *out)
{
    out->x = a->x / b->x;
    out->y = a->y / b->y;
    out->z = a->z / b->z;
    out->w = a->w / b->w;
}

static LX_INLINE void lx_vec4_scale(const lx_vec4_t *a, float s, lx_vec4_t *out)
{
    out->x = a->x * s;
    out->y = a->y * s;
    out->z = a->z * s;
    out->w = a->w * s;
}

static LX_INLINE float lx_vec4_dot(const lx_vec4_t *a, const lx_vec4_t *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}

static LX_INLINE void lx_vec4_cross(const lx_vec4_t *a, const lx_vec4_t *b, lx_vec4_t *out)
{
    out->x = a->y * b->z - a->z * b->y;
    out->y = a->z * b->x - a->x * b->z;
    out->z = a->x * b->y - a->y * b->x;
    out->w = 0.0f;
}

static LX_INLINE float lx_vec4_squared_length(const lx_vec4_t *v)
{
    return lx_vec4_dot(v, v);
}

static LX_INLINE float lx_vec4_length(const lx_vec4_t *v)
{
    return lx_sqrtf(lx_vec4_squared_length(v));
}

static LX_INLINE void lx_vec4_normalize(const lx_vec4_t *v, lx_vec4_t *out)
{
    float length = lx_vec4_squared_length(v);
    if (length > 0.0f) {
        float s = 1.0f / lx_sqrtf(length);
        out->x = v->x * s;
        out->y = v->y * s;
        out->z = v->z * s;
        out->w = v->w * s;
    }
}

/*
 * 4-D Matrix
 */

static LX_INLINE void lx_mat4_zero(lx_mat4_t *m)
{
    *m = (lx_mat4_t) { 0 };
}

static LX_INLINE void lx_mat4_identity(lx_mat4_t *m)
{
    *m = (lx_mat4_t) { 0 };
    m->m11 = m->m22 = m->m33 = m->m44 = 1.0f;
}

static LX_INLINE void lx_mat4_add(const lx_mat4_t *a, const lx_mat4_t *b, lx_mat4_t *out)
{
    out->m11 = a->m11 + b->m11; out->m12 = a->m12 + b->m13; out->m13 = a->m13 + b->m13; out->m14 = a->m14 + b->m14;
    out->m21 = a->m21 + b->m21; out->m22 = a->m22 + b->m23; out->m23 = a->m23 + b->m23; out->m24 = a->m24 + b->m24;
    out->m31 = a->m31 + b->m31; out->m32 = a->m32 + b->m33; out->m33 = a->m33 + b->m33; out->m34 = a->m34 + b->m34;
    out->m41 = a->m41 + b->m41; out->m42 = a->m42 + b->m43; out->m43 = a->m43 + b->m43; out->m44 = a->m44 + b->m44;
}

static LX_INLINE void lx_mat4_sub(const lx_mat4_t *a, const lx_mat4_t *b, lx_mat4_t *out)
{
    out->m11 = a->m11 - b->m11; out->m12 = a->m12 - b->m13; out->m13 = a->m13 - b->m13; out->m14 = a->m14 - b->m14;
    out->m21 = a->m21 - b->m21; out->m22 = a->m22 - b->m23; out->m23 = a->m23 - b->m23; out->m24 = a->m24 - b->m24;
    out->m31 = a->m31 - b->m31; out->m32 = a->m32 - b->m33; out->m33 = a->m33 - b->m33; out->m34 = a->m34 - b->m34;
    out->m41 = a->m41 - b->m41; out->m42 = a->m42 - b->m43; out->m43 = a->m43 - b->m43; out->m44 = a->m44 - b->m44;
}

static LX_INLINE void lx_mat4_mul(const lx_mat4_t *a, const lx_mat4_t *b, lx_mat4_t *out)
{
    out->m11 = a->m11 * b->m11 + a->m12 * b->m21 + a->m13 * b->m31 + a->m14 * b->m41;
    out->m12 = a->m11 * b->m12 + a->m12 * b->m22 + a->m13 * b->m32 + a->m14 * b->m42;
    out->m13 = a->m11 * b->m13 + a->m12 * b->m23 + a->m13 * b->m33 + a->m14 * b->m43;
    out->m14 = a->m11 * b->m14 + a->m12 * b->m24 + a->m13 * b->m34 + a->m14 * b->m44;

    out->m21 = a->m21 * b->m11 + a->m22 * b->m21 + a->m23 * b->m31 + a->m24 * b->m41;
    out->m22 = a->m21 * b->m12 + a->m22 * b->m22 + a->m23 * b->m32 + a->m24 * b->m42;
    out->m23 = a->m21 * b->m13 + a->m22 * b->m23 + a->m23 * b->m33 + a->m24 * b->m43;
    out->m24 = a->m21 * b->m14 + a->m22 * b->m24 + a->m23 * b->m34 + a->m24 * b->m44;

    out->m31 = a->m31 * b->m11 + a->m32 * b->m21 + a->m33 * b->m31 + a->m34 * b->m41;
    out->m32 = a->m31 * b->m12 + a->m32 * b->m22 + a->m33 * b->m32 + a->m34 * b->m42;
    out->m33 = a->m31 * b->m13 + a->m32 * b->m23 + a->m33 * b->m33 + a->m34 * b->m43;
    out->m34 = a->m31 * b->m14 + a->m32 * b->m24 + a->m33 * b->m34 + a->m34 * b->m44;

    out->m41 = a->m41 * b->m11 + a->m42 * b->m21 + a->m43 * b->m31 + a->m44 * b->m41;
    out->m42 = a->m41 * b->m12 + a->m42 * b->m22 + a->m43 * b->m32 + a->m44 * b->m42;
    out->m43 = a->m41 * b->m13 + a->m42 * b->m23 + a->m43 * b->m33 + a->m44 * b->m43;
    out->m44 = a->m41 * b->m14 + a->m42 * b->m24 + a->m43 * b->m34 + a->m44 * b->m44;
}

static LX_INLINE void lx_mat4_scale(const lx_mat4_t *m, float s, lx_mat4_t *out)
{
    for (size_t i = 0; i < 16; ++i)
        out->m[i] = m->m[i] * s;
}

static LX_INLINE void lx_mat4_transpose(const lx_mat4_t *m, lx_mat4_t *out)
{
    out->m11 = m->m11; out->m12 = m->m21; out->m13 = m->m31; out->m14 = m->m41;
    out->m21 = m->m12; out->m22 = m->m22; out->m23 = m->m32; out->m24 = m->m42;
    out->m31 = m->m13; out->m32 = m->m23; out->m33 = m->m33; out->m34 = m->m43;
    out->m41 = m->m14; out->m42 = m->m24; out->m43 = m->m34; out->m44 = m->m44;
}

static LX_INLINE float lx_mat4_det(const lx_mat4_t *m)
{
    float m33_x_m44_minus_m34_x_m43 = m->m33 * m->m44 - m->m34 * m->m43;
    float m23_x_m44_minus_m24_x_m43 = m->m23 * m->m44 - m->m24 * m->m43;
    float m23_x_m34_minus_m24_x_m33 = m->m23 * m->m34 - m->m24 * m->m33;
    float m13_x_m34_minus_m14_x_m33 = m->m13 * m->m34 - m->m14 * m->m33;
    float m13_x_m44_minus_m14_x_m43 = m->m13 * m->m44 - m->m14 * m->m43;
    float m13_x_m24_minus_m14_x_m23 = m->m13 * m->m24 - m->m14 * m->m23;

    return
        m->m11 * (m->m22 * (m33_x_m44_minus_m34_x_m43)
            -m->m32 * (m23_x_m44_minus_m24_x_m43)
            +m->m42 * (m23_x_m34_minus_m24_x_m33))

        - m->m21 * (m->m12 * (m33_x_m44_minus_m34_x_m43)
            -m->m32 * (m13_x_m44_minus_m14_x_m43)
            +m->m42 * (m13_x_m34_minus_m14_x_m33))

        + m->m31 * (m->m12 * (m23_x_m44_minus_m24_x_m43)
            -m->m22 * (m13_x_m44_minus_m14_x_m43)
            +m->m42 * (m13_x_m24_minus_m14_x_m23))

        - m->m41 * (m->m12 * (m23_x_m34_minus_m24_x_m33)
            -m->m22 * (m13_x_m34_minus_m14_x_m33)
            +m->m32 * (m13_x_m24_minus_m14_x_m23));
}

static LX_INLINE float lx_mat4_det3(const lx_mat4_t *m)
{
    return m->m11 * (m->m22 * m->m33 - m->m32 * m->m23)
         - m->m12 * (m->m21 * m->m33 - m->m31 * m->m23)
         + m->m13 * (m->m21 * m->m32 - m->m31 * m->m22);
}

static LX_INLINE bool lx_mat4_inv3(const lx_mat4_t *m, lx_mat4_t *out)
{
    float det = lx_mat4_det3(m);

    if (fabsf(det) < LX_MAT_INVERSE_EPSILON) {
        return false;
    }

    float one_over_det = 1.0f / det;

    out->m11 = one_over_det * (m->m22*m->m33 - m->m23*m->m32);
    out->m12 = one_over_det * (m->m13*m->m32 - m->m12*m->m33);
    out->m13 = one_over_det * (m->m12*m->m23 - m->m13*m->m22);

    out->m21 = one_over_det * (m->m23*m->m31 - m->m21*m->m33);
    out->m22 = one_over_det * (m->m11*m->m33 - m->m13*m->m31);
    out->m23 = one_over_det * (m->m13*m->m21 - m->m11*m->m23);

    out->m31 = one_over_det * (m->m21*m->m32 - m->m22*m->m31);
    out->m32 = one_over_det * (m->m12*m->m31 - m->m11*m->m32);
    out->m33 = one_over_det * (m->m11*m->m22 - m->m12*m->m21);

    return true;
}

static LX_INLINE float lx_mat4_trace(const lx_mat4_t *m)
{
    return m->m11 + m->m22 + m->m33 + m->m44;
}

static LX_INLINE void lx_mat4_set_rotation_x(float angle, lx_mat4_t *out)
{
    float sa = sinf(angle);
    float ca = cosf(angle);

    out->m11 = 1.0f; out->m12 = 0.0f; out->m13 = 0.0f; out->m14 = 0.0f;
    out->m21 = 0.0f; out->m22 = ca;	  out->m23 = sa;   out->m24 = 0.0f;
    out->m31 = 0.0f; out->m32 = -sa;  out->m33 = ca;   out->m34 = 0.0f;
    out->m41 = 0.0f; out->m42 = 0.0f; out->m43 = 0.0f; out->m44 = 1.0f;
}

static LX_INLINE void lx_mat4_set_rotation_y(float angle, lx_mat4_t *out)
{
    float sa = sinf(angle);
    float ca = cosf(angle);

    out->m11 = ca;   out->m12 = 0.0f; out->m13 = -sa;  out->m14 = 0.0f;
    out->m21 = 0.0f; out->m22 = 1.0;  out->m23 = 0.0;  out->m24 = 0.0f;
    out->m31 = sa;   out->m32 = 0.0;  out->m33 = ca;   out->m34 = 0.0f;
    out->m41 = 0.0f; out->m42 = 0.0f; out->m43 = 0.0f; out->m44 = 1.0f;
}

static LX_INLINE void lx_mat4_set_rotation_z(float angle, lx_mat4_t *out)
{
    float sa = sinf(angle);
    float ca = cosf(angle);

    out->m11 = ca;   out->m12 = sa;   out->m13 = 0.0f; out->m14 = 0.0f;
    out->m21 = -sa;  out->m22 = ca;   out->m23 = 0.0f; out->m24 = 0.0f;
    out->m31 = 0.0f; out->m32 = 0.0f; out->m33 = 1.0f; out->m34 = 0.0f;
    out->m41 = 0.0f; out->m42 = 0.0f; out->m43 = 0.0f; out->m44 = 1.0f;
}

/*
 * Set rotation y (yaw), x (pitch) and z (roll).
 */
static LX_INLINE void lx_mat4_set_rotation_yxz(float angle_y, float angle_x, float angle_z, lx_mat4_t *out)
{
    float sy = sinf(angle_y);
    float cy = cosf(angle_y);
    float sx = sinf(angle_x);
    float cx = cosf(angle_x);
    float sz = sinf(angle_z);
    float cz = cosf(angle_z);

    float sy_x_sx = sy*sx;
    float cy_x_sx = cy*sx;

    out->m11 = cy*cz - sy_x_sx*sz;
    out->m12 = cy*sz + sy_x_sx*cz;
    out->m13 = -sy*cx;
    out->m14 = 0.0f;

    out->m21 = -cx*sz;
    out->m22 = cx*cz;
    out->m23 = sx;
    out->m24 = 0.0f;

    out->m31 = sy*cz + cy_x_sx*sz;
    out->m32 = sy*sz - cy_x_sx*cz;
    out->m33 = cy*cx;
    out->m34 = 0.0f;

    out->m41 = 0.0f;
    out->m42 = 0.0f;
    out->m43 = 0.0f;
    out->m44 = 1.0f;
}

static LX_INLINE void lx_mat4_set_translation(float x, float y, float z, lx_mat4_t *out)
{
    out->m11 = 1.0f; out->m12 = 0.0f; out->m13 = 0.0f; out->m14 = 0.0f;
    out->m21 = 0.0f; out->m22 = 1.0f; out->m23 = 0.0f; out->m24 = 0.0f;
    out->m31 = 0.0f; out->m32 = 0.0f; out->m33 = 1.0f; out->m34 = 0.0f;
    out->m41 = x;    out->m42 = y;    out->m43 = z;    out->m44 = 1.0f;
}

static LX_INLINE void lx_mat4_set_translation_from_vec3(const lx_vec3_t *v, lx_mat4_t *out)
{
    lx_mat4_set_translation(v->x, v->y, v->z, out);
}

static LX_INLINE void lx_mat4_set_projection(float near_plane, float far_plane, float width, float height, lx_mat4_t *out)
{
    float two_near = near_plane + near_plane;
    float range = far_plane / (far_plane - near_plane);

    out->m11 = two_near / width;
    out->m12 = 0.0f;
    out->m13 = 0.0f;
    out->m14 = 0.0f;

    out->m21 = 0.0f;
    out->m22 = two_near / height;
    out->m23 = 0.0f;
    out->m24 = 0.0f;

    out->m31 = 0.0f;
    out->m32 = 0.0f;
    out->m33 = range;
    out->m34 = 1.0f;

    out->m41 = 0.0f;
    out->m42 = 0.0f;
    out->m43 = -range * near_plane;
    out->m44 = 0.0f;
}

static LX_INLINE void lx_mat4_set_projection_fov(float near_plane, float far_plane, float fov_y, float aspect_ratio, lx_mat4_t *out)
{
    float half_fov = 0.5f * fov_y;
    float sf = sinf(half_fov);
    float cf = cosf(half_fov);
    float height = cf / sf;
    float width = height / aspect_ratio;
    float range = far_plane / (far_plane - near_plane);

    out->m11 = width;
    out->m12 = 0.0f;
    out->m13 = 0.0f;
    out->m14 = 0.0f;

    out->m21 = 0.0f;
    out->m22 = height;
    out->m23 = 0.0f;
    out->m24 = 0.0f;

    out->m31 = 0.0f;
    out->m32 = 0.0f;
    out->m33 = range;
    out->m34 = 1.0f;

    out->m41 = 0.0f;
    out->m42 = 0.0f;
    out->m43 = -range * near_plane;
    out->m44 = 0.0f;
}

static LX_INLINE lx_mat4_t *lx_mat4_look_at(const lx_vec3_t* at, const lx_vec3_t *eye, const lx_vec3_t *up, lx_mat4_t *out)
{
    // https://msdn.microsoft.com/en-us/library/windows/desktop/bb205342(v=vs.85).aspx

    lx_vec3_t x, y, z;
    lx_vec3_normalize(lx_vec3_sub(at, eye, &z));
    lx_vec3_normalize(lx_vec3_cross(up, &z, &x));
    lx_vec3_cross(&z, &x, &y);

    float dotx = lx_vec3_dot(&x, eye);
    float doty = lx_vec3_dot(&y, eye);
    float dotz = lx_vec3_dot(&z, eye);

    out->m11 = x.x;   out->m12 = y.x;   out->m13 = z.x;   out->m14 = 0.0f;
    out->m21 = x.y;   out->m22 = y.y;   out->m23 = z.y;   out->m24 = 0.0f;
    out->m31 = x.z;   out->m32 = y.z;   out->m33 = z.z;   out->m34 = 0.0f;
    out->m41 = -dotx; out->m42 = -doty; out->m43 = -dotz; out->m44 = 1.0f;

    return out;
}

#ifdef __cplusplus
}
#endif
