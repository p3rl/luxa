#pragma once

#include <luxa/platform.h>
#include <luxa/renderer/gpu.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_render_pipeline_layout {
    lx_allocator_t *allocator;
    VkPipelineLayout handle;
    lx_array_t *shader_ids; // uint32_t
    VkPipelineVertexInputStateCreateInfo vertex_input_state;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineRasterizationStateCreateInfo rasterization_state;
    VkPipelineMultisampleStateCreateInfo multisample_state;
    VkPipelineColorBlendAttachmentState color_blend_attachment_state;
    VkPipelineColorBlendStateCreateInfo color_blend_state;
    bool is_dirty;
} lx_render_pipeline_layout_t;

typedef struct lx_render_pipeline {
    lx_allocator_t *allocator;
    VkPipeline handle;
} lx_render_pipeline_t;

lx_render_pipeline_layout_t *lx_render_pipeline_create_layout(lx_allocator_t *allocator);

void lx_render_pipeline_destroy_layout(lx_gpu_device_t *device, lx_render_pipeline_layout_t *layout);

void lx_render_pipeline_set_viewport_extent(lx_render_pipeline_layout_t *layout, VkExtent2D extent);

void lx_render_pipeline_set_scissor_extent(lx_render_pipeline_layout_t *layout, VkExtent2D extent);

lx_result_t lx_render_pipeline_create(lx_gpu_device_t *device, lx_render_pipeline_layout_t *layout, VkRenderPass render_pass, lx_render_pipeline_t **pipeline);

void lx_render_pipeline_destroy(lx_gpu_device_t *device, lx_render_pipeline_t *pipeline);

lx_result_t lx_render_pipeline_add_shader(lx_render_pipeline_layout_t *layout, uint32_t shader_id);

#ifdef __cplusplus
}
#endif
