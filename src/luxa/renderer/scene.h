#pragma once

#include <luxa/platform.h>
#include <luxa/memory/allocator.h>
#include <luxa/collections/array.h>
#include <luxa/math/math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_scene lx_scene_t;

typedef uint64_t lx_scene_node_t;

typedef enum lx_renderable_type {
    LX_RENDERABLE_TYPE_MESH = 0x00000001
} lx_renderable_type_t;

typedef uint64_t lx_renderable_t;

typedef struct lx_scene_render_data {
    lx_renderable_type_t type;
    lx_renderable_t handle;
    lx_any_t data;
} lx_scene_render_data_t;

static inline lx_scene_node_t lx_nil_scene_node()
{
    return 0;
}

static inline lx_scene_node_t lx_scene_root_node()
{
    return 1;
}

static inline bool lx_is_some_scene_node(uint64_t i)
{
    return i != 0;
}

static inline bool lx_is_nil_scene_node(uint64_t i)
{
    return i == 0;
}

static inline bool lx_is_some_renderable(lx_renderable_t renderable)
{
    return renderable != 0;
}

lx_scene_t *lx_scene_create(lx_allocator_t *allocator);

void lx_scene_destroy(lx_scene_t *scene);

size_t lx_scene_size(const lx_scene_t *scene);

lx_scene_node_t lx_scene_first_child(lx_scene_t *scene, lx_scene_node_t parent);

lx_scene_node_t lx_scene_next_sibling(lx_scene_t *scene, lx_scene_node_t node);

lx_scene_node_t lx_scene_create_node(lx_scene_t *scene, lx_scene_node_t parent);

lx_renderable_t lx_scene_create_renderable(lx_scene_t *scene, lx_renderable_type_t type, lx_any_t render_data);

void lx_scene_attach_renderable(lx_scene_t *scene, lx_scene_node_t node, lx_renderable_t renderable);

lx_renderable_t lx_scene_renderable(lx_scene_t *scene, lx_scene_node_t node);

lx_scene_render_data_t *lx_scene_render_data(lx_scene_t *scene, lx_renderable_t renderable);

const lx_mat4_t *lx_scene_world_transform(const lx_scene_t *scene, lx_scene_node_t node);

#ifdef __cplusplus
}
#endif
