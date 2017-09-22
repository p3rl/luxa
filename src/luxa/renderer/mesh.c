#include <luxa/renderer/mesh.h>
#include <luxa/collections/array.h>

struct lx_mesh {
    lx_allocator_t *allocator;
    lx_array_t *vertices;
    lx_array_t *indices;
};

lx_mesh_t *lx_mesh_create(lx_allocator_t *allocator)
{
    lx_mesh_t *mesh = lx_alloc(allocator, sizeof(lx_mesh_t));
    
    *mesh = (lx_mesh_t) {
        .allocator = allocator,
        .vertices = lx_array_create(allocator, sizeof(lx_vec3_t)),
        .indices = lx_array_create(allocator, sizeof(uint32_t))
    };

    return mesh;
}

void lx_mesh_destroy(lx_mesh_t *mesh)
{
    lx_array_destroy(mesh->vertices);
    lx_array_destroy(mesh->indices);
    lx_free(mesh->allocator, mesh);
}

void lx_mesh_set_vertices(lx_mesh_t *mesh, lx_vec3_t *vertices, size_t num_vertices)
{
    lx_array_copy(mesh->vertices, vertices, num_vertices);
}

void lx_mesh_set_indices(lx_mesh_t *mesh, uint32_t *indices, size_t num_indices)
{
    lx_array_copy(mesh->indices, indices, num_indices);
}