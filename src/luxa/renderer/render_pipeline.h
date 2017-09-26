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
    lx_array_t *vertex_bindings; // VkVertexInputBindingDescription
    lx_array_t *vertex_attributes; // VkVertexInputAttributeDescription
    lx_array_t *descriptor_set_bindings; // VkDescriptorSetLayoutBinding
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineRasterizationStateCreateInfo rasterization_state;
    VkPipelineMultisampleStateCreateInfo multisample_state;
    VkPipelineColorBlendAttachmentState color_blend_attachment_state;
    VkPipelineColorBlendStateCreateInfo color_blend_state;
    VkDescriptorSetLayout descriptor_set_layout;
    bool is_dirty;
} lx_render_pipeline_layout_t;

typedef struct lx_render_pipeline {
    lx_allocator_t *allocator;
    VkPipeline handle;
    VkDescriptorPool descriptor_pool;
    lx_array_t *descriptor_sets;
} lx_render_pipeline_t;

lx_render_pipeline_layout_t *lx_render_pipeline_create_layout(lx_allocator_t *allocator);

void lx_render_pipeline_destroy_layout(lx_gpu_device_t *device, lx_render_pipeline_layout_t *layout);

void lx_render_pipeline_set_viewport_extent(lx_render_pipeline_layout_t *layout, VkExtent2D extent);

void lx_render_pipeline_set_scissor_extent(lx_render_pipeline_layout_t *layout, VkExtent2D extent);

lx_result_t lx_render_pipeline_add_shader(lx_render_pipeline_layout_t *layout, uint32_t shader_id);

void lx_render_pipeline_add_vertex_binding(lx_render_pipeline_layout_t *layout, VkVertexInputBindingDescription *vertex_binding);

void lx_render_pipeline_add_vertex_attribute(lx_render_pipeline_layout_t *layout, VkVertexInputAttributeDescription *attribute);

void lx_render_pipeline_add_descriptor_set_binding(lx_render_pipeline_layout_t *layout, VkDescriptorSetLayoutBinding *descriptor_set_binding);

void lx_render_pipeline_descriptor_pool_sizes(lx_render_pipeline_layout_t *layout, lx_array_t *pool_sizes);

lx_result_t lx_render_pipeline_create(lx_gpu_device_t *device, lx_render_pipeline_layout_t *layout, VkRenderPass render_pass, lx_render_pipeline_t **pipeline);

void lx_render_pipeline_destroy(lx_gpu_device_t *device, lx_render_pipeline_t *pipeline);

#ifdef __cplusplus
}
#endif
