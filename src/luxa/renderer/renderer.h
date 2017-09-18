#ifndef RENDERER_H
#define RENDERER_H

#include <luxa/memory/allocator.h>
#include <luxa/collections/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum lx_shader_type
{
	LX_SHADER_TYPE_VERTEX,
	LX_SHADER_TYPE_FRAGMENT
} lx_shader_type_t;

typedef struct lx_renderer lx_renderer_t;

lx_result_t lx_renderer_create(lx_allocator_t *allocator, lx_renderer_t **renderer, void* window_handle, void* module_handle);

lx_result_t lx_renderer_create_shader(lx_renderer_t *renderer, lx_buffer_t *code, uint32_t id, lx_shader_type_t type);

lx_result_t lx_renderer_create_render_pipelines(lx_renderer_t *renderer, uint32_t vertex_shader_id, uint32_t fragment_shader_id);

void lx_renderer_destroy(lx_allocator_t *allocator, lx_renderer_t *renderer);

void lx_renderer_render_frame(lx_renderer_t *renderer);

#ifdef __cplusplus
}
#endif

#endif // RENDERER_H
