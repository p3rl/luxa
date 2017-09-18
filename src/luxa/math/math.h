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

typedef struct lx_extent2 {
	uint32_t width;
	uint32_t height;
} lx_extent2_t;

#ifdef __cplusplus
}
#endif
