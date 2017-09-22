#include <luxa/renderer/scene.h>
#include <luxa/collections/array.h>
#include <luxa/collections/map.h>
#include <luxa/hash.h>

static const size_t SCENE_NODE_SIZE =
    sizeof(lx_scene_node_t) +   // Parent
    sizeof(lx_scene_node_t) +   // First child
    sizeof(lx_scene_node_t) +   // Next sibling
    sizeof(lx_renderable_t) +   // Renderable
    sizeof(lx_mat4_t);          // Transform

typedef struct buffer {
    void *data;
    size_t size;
    size_t capacity;
} buffer_t;

typedef struct render_data {
    lx_renderable_type_t type;
    lx_renderable_t handle;
    lx_any_t data;
} render_data_t;

struct lx_scene {
    lx_allocator_t *allocator;
    
    // Scene node
    buffer_t buffer;
    lx_scene_node_t *parent;
    lx_scene_node_t *first_child;
    lx_scene_node_t *next_sibling;
    lx_renderable_t *renderable;
    lx_mat4_t *transform;

    lx_array_t *rendera_data; // render_data_t
};

void allocate_buffer(lx_scene_t *scene, size_t capacity)
{
    scene->buffer.capacity = capacity;
    scene->buffer.data = lx_realloc(scene->allocator, scene->buffer.data, capacity * SCENE_NODE_SIZE);

    scene->parent = (lx_scene_node_t *)scene->buffer.data;
    scene->first_child = (lx_scene_node_t *)(scene->parent + scene->buffer.capacity);
    scene->next_sibling = (lx_scene_node_t *)(scene->first_child + scene->buffer.capacity);
    scene->renderable = (lx_renderable_t *)(scene->next_sibling + scene->buffer.capacity);
    scene->transform = (lx_mat4_t *)(scene->renderable + scene->buffer.capacity);
}

lx_scene_t *lx_scene_create(lx_allocator_t *allocator)
{
    const size_t default_capacity = 128;

    lx_scene_t *scene = lx_alloc(allocator, sizeof(lx_scene_t));
    *scene = (lx_scene_t) { 0 };

    scene->allocator = allocator;

    // Init scene node
    allocate_buffer(scene, default_capacity);
    memset(scene->buffer.data, 0, scene->buffer.capacity * SCENE_NODE_SIZE);
    scene->buffer.size = 2;
    lx_mat4_identity(&scene->transform[1]);

    // Init render data
    scene->rendera_data = lx_array_create(allocator, sizeof(render_data_t));
    render_data_t nil_render_data = { 0 };
    lx_array_push_back(scene->rendera_data, &nil_render_data);
    
    return scene;
}

void lx_scene_destroy(lx_scene_t *scene)
{
    lx_array_destroy(scene->rendera_data);
    lx_free(scene->allocator, scene->buffer.data);
    *scene = (lx_scene_t) { 0 };
}

lx_scene_node_t lx_scene_first_child(lx_scene_t *scene, lx_scene_node_t parent)
{
    LX_ASSERT(lx_is_some_scene_node(parent), "Invaild parent");
    return scene->first_child[parent];
}

lx_scene_node_t lx_scene_next_sibling(lx_scene_t *scene, lx_scene_node_t node)
{
    LX_ASSERT(lx_is_some_scene_node(node), "Invalid node");
    return scene->next_sibling[node];
}

lx_scene_node_t lx_scene_create_node(lx_scene_t *scene, lx_scene_node_t parent)
{
    LX_ASSERT(lx_is_some_scene_node(parent), "Invalid parent");
    
    if (scene->buffer.size >= scene->buffer.capacity)
        allocate_buffer(scene, scene->buffer.capacity * 2);
    
    lx_scene_node_t node = scene->buffer.size;
    scene->buffer.size++;

    scene->parent[node] = parent;
    scene->first_child[node] = 0;
    scene->next_sibling[node] = 0;
    scene->renderable[node] = 0;
    lx_mat4_identity(&scene->transform[node]);

    lx_scene_node_t first_child = scene->first_child[parent];
    if (lx_is_nil_scene_node(first_child)) {
        scene->first_child[parent] = node;
        return node;
    }

    lx_scene_node_t sibling = first_child;
    while (scene->next_sibling[sibling]) {
        sibling = scene->next_sibling[sibling];
    }
    scene->next_sibling[sibling] = node;
    
    return node;
}

lx_renderable_t lx_scene_create_renderable(lx_scene_t *scene, lx_renderable_type_t type, lx_any_t render_data)
{
    LX_ASSERT(type == LX_RENDERABLE_TYPE_MESH, "Only meshes is allowed");

    lx_renderable_t handle = lx_array_size(scene->rendera_data);
    render_data_t rendera_data = { .handle = handle,.type = type, .data = render_data };
    lx_array_push_back(scene->rendera_data, &rendera_data);
    return handle;
}

void lx_scene_attach_renderable(lx_scene_t *scene, lx_scene_node_t node, lx_renderable_t renderable)
{
    LX_ASSERT(scene, "Invalid scene");
    LX_ASSERT(lx_is_some_scene_node(node), "Invalid scene node");

    scene->renderable[node] = renderable;
}