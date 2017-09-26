#pragma once

#include <luxa/platform.h>
#include <luxa/memory/allocator.h>
#include <luxa/math/math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_camera {
    lx_allocator_t *allocator;
    lx_vec3_t position;
    lx_vec3_t direction;
    lx_vec3_t up;
    float near_plane;
    float far_plane;
    float fov;
} lx_camera_t;

lx_camera_t *lx_camera_create(lx_allocator_t *alloator);

void lx_camera_destroy(lx_camera_t *camera);

void lx_camera_set_projection(lx_camera_t *camera, float near_plane, float far_plane, float fov);

void lx_camera_look_to(lx_camera_t *camera, lx_vec3_t *direction, lx_vec3_t *position, lx_vec3_t *up);

void lx_camera_look_at(lx_camera_t *camera, lx_vec3_t *target, lx_vec3_t *position, lx_vec3_t *up);

#ifdef __cplusplus
}
#endif
