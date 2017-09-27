#include <luxa/renderer/camera.h>

lx_camera_t *lx_camera_create(lx_allocator_t *allocator)
{
    LX_ASSERT(allocator, "Invalid allocator");

    lx_camera_t *camera = lx_alloc(allocator, sizeof(lx_camera_t));
    *camera = (lx_camera_t) { 0 };
    camera->allocator = allocator;
    return camera;
}

void lx_camera_destroy(lx_camera_t *camera)
{
    LX_ASSERT(camera, "Invalid camera");

    lx_free(camera->allocator, camera);
}

void lx_camera_set_projection(lx_camera_t *camera, float near_plane, float far_plane, float fov)
{
    camera->near_plane = near_plane;
    camera->far_plane = far_plane;
    camera->fov = fov;
}

void lx_camera_look_to(lx_camera_t *camera, lx_vec3_t *direction, lx_vec3_t *position, lx_vec3_t *up)
{
    camera->position = *position;
    camera->direction = *direction;
    camera->up = *up;
}

void lx_camera_look_at(lx_camera_t *camera, lx_vec3_t *target, lx_vec3_t *position, lx_vec3_t *up)
{
    lx_vec3_t dir;
    lx_camera_look_to(camera, lx_vec3_sub(target, position, &dir), position, up);
}
