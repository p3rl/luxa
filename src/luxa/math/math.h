#pragma once

#include <luxa/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_vec2 {
	float x;
	float y;
} lx_vec2_t;

typedef struct lx_vec3 {
	float x;
	float y;
	float z;
} lx_vec3_t;

typedef struct lx_vec4 {
	float x;
	float y;
	float z;
	float w;
} lx_vec4_t;

typedef struct lx_quat {
    float x;
    float y;
    float z;
    float w;
} lx_quat_t;

typedef struct lx_mat4 {
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

static inline void lx_mat4_identity(lx_mat4_t *m)
{
    *m = (lx_mat4_t) { 0 };
    m->m11 = m->m22 = m->m33 = m->m44 = 1.0f;
}

#ifdef __cplusplus
}
#endif
