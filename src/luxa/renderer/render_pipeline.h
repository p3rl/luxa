#pragma once

#include <luxa/platform.h>
#include <luxa/renderer/gpu.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_render_pipeline_layout lx_render_pipeline_layout_t;

typedef struct lx_render_pipeline lx_render_pipeline_t;

lx_render_pipeline_layout_t *lx_render_pipeline_create_layout(lx_allocator_t *allocator);

void lx_render_pipeline_destroy_layout(lx_render_pipeline_layout_t *layout);

lx_result_t lx_render_pipeline_create(lx_gpu_device_t *device, lx_render_pipeline_layout_t *layout, VkRenderPass render_pass, lx_render_pipeline_t **pipeline);

void lx_render_pipeline_destroy(lx_gpu_device_t *device, lx_render_pipeline_t *pipeline);

void lx_render_pipeline_destroy_pipeline(lx_gpu_device_t *device, lx_render_pipeline_t *pipeline);

lx_result_t lx_render_pipeline_add_shader(lx_render_pipeline_layout_t *layout, uint32_t shader_id);

#ifdef __cplusplus
}
#endif
