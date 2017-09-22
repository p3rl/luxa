#pragma once

#include <luxa/platform.h>
#include <luxa/memory/allocator.h>
#include <luxa/math/math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_mesh lx_mesh_t;

lx_mesh_t *lx_mesh_create(lx_allocator_t *allocator);

void lx_mesh_destroy(lx_mesh_t *mesh);

void lx_mesh_set_vertices(lx_mesh_t *mesh, lx_vec3_t *vertices, size_t num_vertices);

void lx_mesh_set_indices(lx_mesh_t *mesh, uint32_t *indices, size_t num_indices);

#ifdef __cplusplus
}
#endif
