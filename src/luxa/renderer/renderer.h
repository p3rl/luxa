#pragma once

#include <luxa/memory/allocator.h>
#include <luxa/collections/buffer.h>
#include <luxa/math/math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_renderer lx_renderer_t;

lx_result_t lx_renderer_create(lx_allocator_t *allocator, lx_renderer_t **renderer, void* window_handle, lx_extent2_t window_size, void* module_handle);

lx_result_t lx_renderer_create_shader(lx_renderer_t *renderer, lx_buffer_t *code, uint32_t id);

lx_result_t lx_renderer_create_render_pipelines(lx_renderer_t *renderer, uint32_t vertex_shader_id, uint32_t fragment_shader_id);

void lx_renderer_destroy(lx_allocator_t *allocator, lx_renderer_t *renderer);

void lx_renderer_render_frame(lx_renderer_t *renderer);

void lx_renderer_device_wait_idle(lx_renderer_t *renderer);

lx_result_t lx_renderer_reset_swap_chain(lx_renderer_t *renderer, lx_extent2_t swap_chain_extent, uint32_t vertex_shader_id, uint32_t fragment_shader_id);

#ifdef __cplusplus
}
#endif
