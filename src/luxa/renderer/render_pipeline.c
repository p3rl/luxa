#include <luxa/renderer/render_pipeline.h>
#include <vulkan/vulkan.h>

bool is_pool_size_type(VkDescriptorPoolSize *pool_size, VkDescriptorType *descriptor_type)
{
    return pool_size->type == *descriptor_type;
}

lx_render_pipeline_layout_t *lx_render_pipeline_create_layout(lx_allocator_t *allocator)
{
    LX_ASSERT(allocator, "Invaild allocator");

    lx_render_pipeline_layout_t *layout = lx_alloc(allocator, sizeof(lx_render_pipeline_layout_t));
    *layout = (lx_render_pipeline_layout_t) { 0 };
    
    layout->allocator = allocator;
    layout->shader_ids = lx_array_create(allocator, sizeof(uint32_t));
    layout->vertex_bindings = lx_array_create(allocator, sizeof(VkVertexInputBindingDescription));
    layout->vertex_attributes = lx_array_create(allocator, sizeof(VkVertexInputAttributeDescription));
    layout->descriptor_set_bindings = lx_array_create(allocator, sizeof(VkDescriptorSetLayoutBinding));

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
    layout->rasterization_state.cullMode = VK_CULL_MODE_NONE;
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

	layout->depth_stencil_state = (VkPipelineDepthStencilStateCreateInfo) { 0 };
	layout->depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	layout->depth_stencil_state.depthTestEnable = VK_TRUE;
	layout->depth_stencil_state.depthWriteEnable = VK_TRUE;
	layout->depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS;
	layout->depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
	layout->depth_stencil_state.minDepthBounds = 0.0f; // Optional
	layout->depth_stencil_state.maxDepthBounds = 1.0f; // Optional
	layout->depth_stencil_state.stencilTestEnable = VK_FALSE;
	layout->depth_stencil_state.front = (VkStencilOpState) { 0 }; // Optional
	layout->depth_stencil_state.back = (VkStencilOpState) { 0 }; // Optional
    
    layout->is_dirty = true;

    return layout;
}

void lx_render_pipeline_destroy_layout(lx_gpu_device_t *device, lx_render_pipeline_layout_t *layout)
{
    LX_ASSERT(device, "Invalid layout");
    LX_ASSERT(layout, "Invalid layout");
    LX_ASSERT(layout->allocator, "Invalid layout allocator");
    
    if (layout->handle != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device->handle, layout->handle, NULL);
    }

    if (layout->descriptor_set_layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device->handle, layout->descriptor_set_layout, NULL);
    }
    
    lx_array_destroy(layout->shader_ids);
    lx_array_destroy(layout->vertex_bindings);
    lx_array_destroy(layout->vertex_attributes);
    lx_free(layout->allocator, layout);
}

void lx_render_pipeline_set_viewport_extent(lx_render_pipeline_layout_t *layout, VkExtent2D extent)
{
    LX_ASSERT(layout, "Invalid layout");

    layout->viewport.width = (float)extent.width;
    layout->viewport.height = (float)extent.height;
    layout->is_dirty = true;
}

void lx_render_pipeline_set_scissor_extent(lx_render_pipeline_layout_t *layout, VkExtent2D extent)
{
    LX_ASSERT(layout, "Invalid layout");

    layout->scissor.extent = extent;
    layout->is_dirty = true;
}

void lx_render_pipeline_add_vertex_binding(lx_render_pipeline_layout_t *layout, VkVertexInputBindingDescription *vertex_binding)
{
    LX_ASSERT(layout, "Invalid layout");

    lx_array_push_back(layout->vertex_bindings, vertex_binding);
    layout->is_dirty = true;
}

void lx_render_pipeline_add_vertex_attribute(lx_render_pipeline_layout_t *layout, VkVertexInputAttributeDescription *attribute)
{
    LX_ASSERT(layout, "Invalid layout");
    
    lx_array_push_back(layout->vertex_attributes, attribute);
}

void lx_render_pipeline_add_descriptor_set_binding(lx_render_pipeline_layout_t *layout, VkDescriptorSetLayoutBinding *descriptor_set_binding)
{
    LX_ASSERT(layout, "Invalid layout");

    lx_array_push_back(layout->descriptor_set_bindings, descriptor_set_binding);
}

void lx_render_pipeline_descriptor_pool_sizes(lx_render_pipeline_layout_t *layout, lx_array_t *pool_sizes)
{
    LX_ASSERT(layout, "Invalid layout");
    LX_ASSERT(layout->descriptor_set_bindings, "Invalid descriptor set bindings");
    LX_ASSERT(pool_sizes, "Invalid pool sizes");
    
    lx_array_for(VkDescriptorSetLayoutBinding, db, layout->descriptor_set_bindings) {
        VkDescriptorPoolSize *pool_size = lx_array_find_if(pool_sizes, is_pool_size_type, &db->descriptorType);
        if (pool_size) {
            pool_size->descriptorCount++;
        }
        else {
            VkDescriptorPoolSize ps = { .type = db->descriptorType, .descriptorCount = 1 };
            lx_array_push_back(pool_sizes, &ps);
        }
    }
}

lx_result_t lx_render_pipeline_create(lx_gpu_device_t *device, lx_render_pipeline_layout_t *layout, VkRenderPass render_pass, lx_render_pipeline_t **pipeline)
{
    // Check if layout has changed and needs to be recreated
    if (layout->is_dirty && layout->handle != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device->handle, layout->handle, NULL);
        layout->handle = VK_NULL_HANDLE;

        if (layout->descriptor_set_layout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device->handle, layout->descriptor_set_layout, NULL);
            layout->descriptor_set_layout = NULL;
        }
    }

    if (layout->handle == VK_NULL_HANDLE) {
        VkPipelineLayoutCreateInfo pipeline_layout_create_info = { 0 };
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.pushConstantRangeCount = 0;

        // Create descriptor set bindings
        size_t num_descriptor_bindings = lx_array_size(layout->descriptor_set_bindings);
        
        if (num_descriptor_bindings) {
            VkDescriptorSetLayoutCreateInfo descriptor_set_create_info = { 0 };
            descriptor_set_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptor_set_create_info.bindingCount = (uint32_t)num_descriptor_bindings;
            descriptor_set_create_info.pBindings = lx_array_begin(layout->descriptor_set_bindings);
            
            if (vkCreateDescriptorSetLayout(device->handle, &descriptor_set_create_info, NULL, &layout->descriptor_set_layout) != VK_SUCCESS) {
                return LX_ERROR;
            }
            
            pipeline_layout_create_info.setLayoutCount = 1;
            pipeline_layout_create_info.pSetLayouts = &layout->descriptor_set_layout;
        }

        if (vkCreatePipelineLayout(device->handle, &pipeline_layout_create_info, NULL, &layout->handle) != VK_SUCCESS) {
            return LX_ERROR;
        }

        layout->is_dirty = false;
    }

    // Create descriptor pool
    lx_array_t *pool_sizes = lx_array_create(layout->allocator, sizeof(VkDescriptorPoolSize));
    lx_render_pipeline_descriptor_pool_sizes(layout, pool_sizes);
    size_t num_pool_sizes = lx_array_size(pool_sizes);

    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;

    if (num_pool_sizes) {
        VkDescriptorPoolCreateInfo descriptor_pool_create_info = { 0 };
        descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.poolSizeCount = (uint32_t)num_pool_sizes;
        descriptor_pool_create_info.pPoolSizes = lx_array_begin(pool_sizes);
        descriptor_pool_create_info.maxSets = 1;
        
        if (vkCreateDescriptorPool(device->handle, &descriptor_pool_create_info, NULL, &descriptor_pool) != VK_SUCCESS) {
            lx_array_destroy(pool_sizes);
            pool_sizes = NULL;
            return LX_ERROR;
        }

        VkDescriptorSetAllocateInfo descriptor_set_alloc_info = { 0 };
        descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_alloc_info.descriptorPool = descriptor_pool;
        descriptor_set_alloc_info.descriptorSetCount = 1;
        descriptor_set_alloc_info.pSetLayouts = &layout->descriptor_set_layout;

        if (vkAllocateDescriptorSets(device->handle, &descriptor_set_alloc_info, &descriptor_set) != VK_SUCCESS) {
            lx_array_destroy(pool_sizes);
            pool_sizes = NULL;
            return LX_ERROR;
        }
    }

    lx_array_destroy(pool_sizes);
    pool_sizes = NULL;

    // Create shader stages
    size_t num_stages = lx_array_size(layout->shader_ids);
    VkPipelineShaderStageCreateInfo *shader_stages = lx_alloc(layout->allocator, sizeof(VkPipelineShaderStageCreateInfo) * num_stages);
    
    for (size_t i = 0; i < num_stages; ++i) {
        uint32_t *shader_id = lx_array_at(layout->shader_ids, i);
        lx_shader_t *shader = lx_gpu_shader(device, *shader_id);
        LX_ASSERT(shader, "Shader does not exists");
        shader_stages[i] = (VkPipelineShaderStageCreateInfo) { 0 };
        shader_stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[i].module = shader->handle;
        shader_stages[i].stage = shader->stage;
        shader_stages[i].pName = "main";
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_state = { 0 };
    vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state.pVertexBindingDescriptions = lx_array_begin(layout->vertex_bindings);
    vertex_input_state.vertexBindingDescriptionCount = (uint32_t)lx_array_size(layout->vertex_bindings);
    vertex_input_state.pVertexAttributeDescriptions = lx_array_begin(layout->vertex_attributes);
    vertex_input_state.vertexAttributeDescriptionCount = (uint32_t)lx_array_size(layout->vertex_attributes);

    VkPipelineViewportStateCreateInfo viewport_state_info = { 0 };
    viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_info.viewportCount = 1;
    viewport_state_info.pViewports = &layout->viewport;
    viewport_state_info.scissorCount = 1;
    viewport_state_info.pScissors = &layout->scissor;
    
    VkGraphicsPipelineCreateInfo pipeline_create_info = { 0 };
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = (uint32_t)num_stages;
    pipeline_create_info.pStages = shader_stages;
    pipeline_create_info.pVertexInputState = &vertex_input_state;
    pipeline_create_info.pInputAssemblyState = &layout->input_assembly_state;
    pipeline_create_info.pViewportState = &viewport_state_info;
    pipeline_create_info.pRasterizationState = &layout->rasterization_state;
    pipeline_create_info.pMultisampleState = &layout->multisample_state;
    pipeline_create_info.pColorBlendState = &layout->color_blend_state;
	pipeline_create_info.pDepthStencilState = &layout->depth_stencil_state;
    pipeline_create_info.layout = layout->handle;
    pipeline_create_info.renderPass = render_pass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline pipeline_handle;
    VkResult result = vkCreateGraphicsPipelines(device->handle, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &pipeline_handle);
    
    if (result == VK_SUCCESS) {
        *pipeline = lx_alloc(layout->allocator, sizeof(lx_render_pipeline_t));
        **pipeline = (lx_render_pipeline_t) {
            .allocator = layout->allocator,
            .handle = pipeline_handle,
            .descriptor_pool = descriptor_pool,
            .descriptor_set = descriptor_set
        };
    }

    lx_free(layout->allocator, shader_stages);
    
    return result;
}

void lx_render_pipeline_destroy(lx_gpu_device_t *device, lx_render_pipeline_t *pipeline)
{
    LX_ASSERT(device, "Invalid device");
    LX_ASSERT(pipeline, "Invalid pipeline");

    if (pipeline->descriptor_pool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device->handle, pipeline->descriptor_pool, NULL);
    }

    if (pipeline->handle != VK_NULL_HANDLE) {
        vkDestroyPipeline(device->handle, pipeline->handle, NULL);
    }

    lx_free(pipeline->allocator, pipeline);
}

lx_result_t lx_render_pipeline_add_shader(lx_render_pipeline_layout_t *layout, uint32_t shader_id)
{
    LX_ASSERT(layout, "Invalid layout");
    lx_array_push_back(layout->shader_ids, &shader_id);
    return LX_SUCCESS;
}
