#include <luxa/renderer/render_pipeline.h>
#include <vulkan/vulkan.h>

struct lx_render_pipeline_layout {
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
};

struct lx_render_pipeline {
    lx_render_pipeline_layout_t *layout;
    VkPipeline handle;
}; 
lx_render_pipeline_layout_t *lx_render_pipeline_create_layout(lx_allocator_t *allocator)
{
    LX_ASSERT(allocator, "Invaild allocator");

    lx_render_pipeline_layout_t *layout = lx_alloc(allocator, sizeof(lx_render_pipeline_layout_t));
    *layout = (lx_render_pipeline_layout_t) { 0 };
    
    layout->allocator = allocator;
    layout->shader_ids = lx_array_create(allocator, sizeof(uint32_t));

    layout->vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    layout->vertex_input_state.vertexBindingDescriptionCount = 0;
    layout->vertex_input_state.vertexAttributeDescriptionCount = 0;

    layout->input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    layout->input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    layout->input_assembly_state.primitiveRestartEnable = VK_FALSE;

    layout->viewport.x = 0.0f;
    layout->viewport.y = 0.0f;
    layout->viewport.width = 0.0f;
    layout->viewport.height = 0.0f;
    layout->viewport.minDepth = 0.0f;
    layout->viewport.maxDepth = 1.0f;

    layout->rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    layout->rasterization_state.depthClampEnable = VK_FALSE;
    layout->rasterization_state.rasterizerDiscardEnable = VK_FALSE;
    layout->rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
    layout->rasterization_state.lineWidth = 1.0f;
    layout->rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
    layout->rasterization_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
    layout->rasterization_state.depthBiasEnable = VK_FALSE;

    layout->multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    layout->multisample_state.sampleShadingEnable = VK_FALSE;
    layout->multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    layout->color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    layout->color_blend_attachment_state.blendEnable = VK_FALSE;

    layout->color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    layout->color_blend_state.logicOpEnable = VK_FALSE;
    layout->color_blend_state.logicOp = VK_LOGIC_OP_COPY;
    layout->color_blend_state.attachmentCount = 1;
    layout->color_blend_state.pAttachments = &layout->color_blend_attachment_state;
    layout->color_blend_state.blendConstants[0] = 0.0f;
    layout->color_blend_state.blendConstants[1] = 0.0f;
    layout->color_blend_state.blendConstants[2] = 0.0f;
    layout->color_blend_state.blendConstants[3] = 0.0f;

    return layout;
}

void lx_render_pipeline_destroy_layout(lx_render_pipeline_layout_t *layout)
{
    LX_ASSERT(layout, "Invalid layout");
    LX_ASSERT(layout->allocator, "Invalid layout allocator");
    lx_free(layout->allocator, layout);
}

lx_result_t lx_render_pipeline_create(lx_gpu_device_t *device, lx_render_pipeline_layout_t *layout, VkRenderPass render_pass, lx_render_pipeline_t **pipeline)
{
    if (layout->handle == VK_NULL_HANDLE) {
        VkPipelineLayoutCreateInfo pipeline_layout_create_info = { 0 };
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = 0;
        pipeline_layout_create_info.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(device->handle, &pipeline_layout_create_info, NULL, &layout->handle) != VK_SUCCESS) {
            return LX_ERROR;
        }
    }

    // Create shader stages
    size_t num_shaders = lx_array_size(layout->shader_ids);
    VkPipelineShaderStageCreateInfo *shader_stages = lx_alloc(layout->allocator, sizeof(VkPipelineShaderStageCreateInfo) * num_shaders);
    
    for (size_t i = 0; i < num_shaders; ++i) {
        uint32_t *shader_id = lx_array_at(layout->shader_ids, i);
        lx_shader_t *shader = lx_gpu_shader(device, *shader_id);
        LX_ASSERT(shader, "Shader does not exists");

        shader_stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[i].module = shader->handle;
        shader_stages[i].stage = shader->stage;
        shader_stages[i].pName = "main";
    }
    
    VkGraphicsPipelineCreateInfo pipeline_create_info = { 0 };
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = 0;
    pipeline_create_info.pStages = NULL;
    pipeline_create_info.pVertexInputState = &layout->vertex_input_state;
    pipeline_create_info.pInputAssemblyState = &layout->input_assembly_state;
    pipeline_create_info.pViewportState = NULL;
    pipeline_create_info.pRasterizationState = &layout->rasterization_state;
    pipeline_create_info.pMultisampleState = &layout->multisample_state;
    pipeline_create_info.pColorBlendState = &layout->color_blend_state;
    pipeline_create_info.layout = layout->handle;
    pipeline_create_info.renderPass = render_pass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline pipeline_handle;
    VkResult result = vkCreateGraphicsPipelines(device->handle, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &pipeline_handle);
    
    if (result == VK_SUCCESS) {
        *pipeline = lx_alloc(layout->allocator, sizeof(lx_render_pipeline_t));
        **pipeline = (lx_render_pipeline_t) { .layout = layout, .handle = pipeline_handle };
    }

    lx_free(layout->allocator, shader_stages);
    
    return result;
}

void lx_render_pipeline_destroy_pipeline(lx_gpu_device_t *device, lx_render_pipeline_t *pipeline)
{
    vkDestroyPipeline(device->handle, pipeline->handle, NULL);
    pipeline->handle = VK_NULL_HANDLE;
}

void lx_render_pipeline_destroy(lx_gpu_device_t *device, lx_render_pipeline_t *pipeline)
{
    LX_ASSERT(device, "Invalid device");
    LX_ASSERT(pipeline, "Invalid pipeline");

    lx_render_pipeline_destroy_layout(pipeline->layout);
    lx_render_pipeline_destroy_pipeline(device, pipeline);
}

lx_result_t lx_render_pipeline_add_shader(lx_render_pipeline_layout_t *layout, uint32_t shader_id)
{
    LX_ASSERT(layout, "Invalid layout");
    lx_array_push_back(layout->shader_ids, &shader_id);
    return LX_SUCCESS;
}
