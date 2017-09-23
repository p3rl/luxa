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

size_t lx_mesh_num_vertices(const lx_mesh_t *mesh);

size_t lx_mesh_num_indices(const lx_mesh_t *mesh);

size_t lx_mesh_vertices_byte_size(const lx_mesh_t *mesh);

size_t lx_mesh_indices_byte_size(const lx_mesh_t *mesh);

const lx_vec3_t *lx_mesh_vertices(const lx_mesh_t *mesh);

const uint32_t *lx_mesh_indices(const lx_mesh_t *mesh);

void lx_mesh_set_vertices(lx_mesh_t *mesh, lx_vec3_t *vertices, size_t num_vertices);

void lx_mesh_set_indices(lx_mesh_t *mesh, uint32_t *indices, size_t num_indices);

void lx_mesh_set_vertex_buffer(lx_mesh_t *mesh, lx_any_t vertex_buffer);

lx_any_t lx_mesh_vertex_buffer(const lx_mesh_t *mesh);

void lx_mesh_set_index_buffer(lx_mesh_t *mesh, lx_any_t index_buffer);

lx_any_t lx_mesh_index_buffer(const lx_mesh_t *mesh);

#ifdef __cplusplus
}
#endif
