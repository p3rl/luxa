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

static LX_INLINE void lx_vec3_sub(const lx_vec3_t *a, const lx_vec3_t *b, lx_vec3_t *out)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
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

static LX_INLINE void lx_vec3_cross(const lx_vec3_t *a, const lx_vec3_t *b, lx_vec3_t *out)
{
    out->x = a->y * b->z - a->z * b->y;
    out->y = a->z * b->x - a->x * b->z;
    out->z = a->x * b->y - a->y * b->x;
}

static LX_INLINE float lx_vec3_squared_length(const lx_vec3_t *v)
{
    return lx_vec3_dot(v, v);
}

static LX_INLINE float lx_vec3_length(const lx_vec3_t *v)
{
    return lx_sqrtf(lx_vec3_squared_length(v));
}

static LX_INLINE void lx_vec3_normalize(const lx_vec3_t *v, lx_vec3_t *out)
{
    float length = lx_vec3_squared_length(v);
    if (length > 0.0f) {
        float s = 1.0f / lx_sqrtf(length);
        out->x = v->x * s;
        out->y = v->y * s;
        out->z = v->z * s;
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

static LX_INLINE void lx_mat4_identity(lx_mat4_t *m)
{
    *m = (lx_mat4_t) { 0 };
    m->m11 = m->m22 = m->m33 = m->m44 = 1.0f;
}

#ifdef __cplusplus
}
#endif
