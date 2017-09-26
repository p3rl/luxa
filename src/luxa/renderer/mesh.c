#include <luxa/renderer/mesh.h>
#include <luxa/collections/array.h>

struct lx_mesh {
    lx_allocator_t *allocator;
    lx_array_t *vertices; // lx_vec2_t
    lx_array_t *indices; // uint32_t
    lx_any_t vertex_buffer;
    lx_any_t index_buffer;
};

lx_mesh_t *lx_mesh_create(lx_allocator_t *allocator)
{
    lx_mesh_t *mesh = lx_alloc(allocator, sizeof(lx_mesh_t));
    
    *mesh = (lx_mesh_t) {
        .allocator = allocator,
        .vertices = lx_array_create(allocator, sizeof(lx_vec2_t)),
        .indices = lx_array_create(allocator, sizeof(uint32_t)),
        .vertex_buffer = NULL,
        .index_buffer = NULL
    };

    return mesh;
}

void lx_mesh_destroy(lx_mesh_t *mesh)
{
    lx_array_destroy(mesh->vertices);
    lx_array_destroy(mesh->indices);
    lx_free(mesh->allocator, mesh);
}

size_t lx_mesh_num_vertices(const lx_mesh_t *mesh)
{
    return lx_array_size(mesh->vertices);
}

size_t lx_mesh_num_indices(const lx_mesh_t *mesh)
{
    return lx_array_size(mesh->indices);
}

const lx_vec2_t *lx_mesh_vertices(const lx_mesh_t *mesh)
{
    return lx_array_begin(mesh->vertices);
}

size_t lx_mesh_vertices_byte_size(const lx_mesh_t *mesh)
{
    return lx_array_size(mesh->vertices) * sizeof(lx_vec2_t);
}

size_t lx_mesh_indices_byte_size(const lx_mesh_t *mesh)
{
    return lx_array_size(mesh->indices) * sizeof(uint32_t);
}

const uint32_t *lx_mesh_indices(const lx_mesh_t *mesh)
{
    return lx_array_begin(mesh->indices);
}

void lx_mesh_set_vertices(lx_mesh_t *mesh, lx_vec2_t *vertices, size_t num_vertices)
{
    lx_array_copy(mesh->vertices, vertices, num_vertices);
}

void lx_mesh_set_indices(lx_mesh_t *mesh, uint32_t *indices, size_t num_indices)
{
    lx_array_copy(mesh->indices, indices, num_indices);
}

void lx_mesh_set_vertex_buffer(lx_mesh_t *mesh, lx_any_t vertex_buffer)
{
    mesh->vertex_buffer = vertex_buffer;
}

void lx_mesh_set_index_buffer(lx_mesh_t *mesh, lx_any_t index_buffer)
{
    mesh->index_buffer = index_buffer;
}

lx_any_t lx_mesh_vertex_buffer(const lx_mesh_t *mesh)
{
    return mesh->vertex_buffer;
}

lx_any_t lx_mesh_index_buffer(const lx_mesh_t *mesh)
{
    return mesh->index_buffer;
}