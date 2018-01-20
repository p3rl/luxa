#include <luxa/renderer/gpu.h>
#include <luxa/log.h>

static const char *LOG_TAG = "GPU";

static bool shader_id_equals(lx_shader_t *shader, lx_any_t id)
{
    return shader->id == *(uint32_t*)id;
}

uint32_t find_memory_type_index(lx_gpu_device_t *device, VkMemoryRequirements *memory_requirements, VkMemoryPropertyFlags memory_properties)
{
    uint32_t memory_type = memory_requirements->memoryTypeBits;
    for (uint32_t i = 0; i < device->gpu->memory_properties.memoryTypeCount; ++i) {
        if ((memory_type & (1 << i)) && (device->gpu->memory_properties.memoryTypes[i].propertyFlags & memory_properties) == memory_properties) {
            return i;
        }
    }
    
    LX_ASSERT(1, "Unable to find memory type index");
    return UINT32_MAX;
}

lx_result_t lx_gpu_all_available(lx_allocator_t *allocator, VkInstance instance, VkSurfaceKHR presentation_surface, lx_array_t **gpus)
{
    LX_ASSERT(allocator, "Invalid allocator");
    LX_ASSERT(instance, "Invalid Vulkan Instance");
    LX_ASSERT(gpus, "Invalid gpu array");

    uint32_t num_devices = 0;
    vkEnumeratePhysicalDevices(instance, &num_devices, NULL);
    if (!num_devices) {
        LX_LOG_ERROR(LOG_TAG, "No gpu(s) found");
        return LX_ERROR;
    }

    VkPhysicalDevice *device_handles = lx_alloc(allocator, sizeof(VkPhysicalDevice) * num_devices);

    if (vkEnumeratePhysicalDevices(instance, &num_devices, device_handles) != VK_SUCCESS) {
        LX_LOG_ERROR(LOG_TAG, "Failed to enumerate gpu(s)");
        lx_free(allocator, device_handles);
        return LX_ERROR;
    }

    *gpus = lx_array_create(allocator, sizeof(lx_gpu_t));

    for (uint32_t i = 0; i < num_devices; ++i) {
        lx_gpu_t gpu = { 0 };
        gpu.allocator = allocator;
        gpu.handle = device_handles[i];
        gpu.compute_queue_family_index = UINT32_MAX;
        gpu.graphics_queue_family_index = UINT32_MAX;
        gpu.presentation_queue_family_index = UINT32_MAX;

        vkGetPhysicalDeviceProperties(gpu.handle, &gpu.properties);
        vkGetPhysicalDeviceFeatures(gpu.handle, &gpu.features);
        vkGetPhysicalDeviceMemoryProperties(gpu.handle, &gpu.memory_properties);

        uint32_t num_queue_properties = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu.handle, &num_queue_properties, NULL);

        if (num_queue_properties) {
            gpu.queue_family_properties = lx_array_create_with_size(allocator, sizeof(VkQueueFamilyProperties), num_queue_properties);
            vkGetPhysicalDeviceQueueFamilyProperties(gpu.handle, &num_queue_properties, lx_array_begin(gpu.queue_family_properties));

            // Setup queue family indices
            for (uint32_t j = 0; j < num_queue_properties; ++j) {
                VkQueueFamilyProperties *queue_properties = lx_array_at(gpu.queue_family_properties, j);
                if (queue_properties->queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    gpu.graphics_queue_family_index = j;
                }
                else if (queue_properties->queueFlags & VK_QUEUE_COMPUTE_BIT) {
                    gpu.compute_queue_family_index = j;
                }

                VkBool32 present_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(gpu.handle, j, presentation_surface, &present_support);
                if (present_support) {
                    gpu.presentation_queue_family_index = j;
                }
            }
        }

        lx_array_push_back(*gpus, &gpu);
    }

    lx_free(allocator, device_handles);

    return LX_SUCCESS;
}

lx_result_t lx_gpu_create_device(lx_gpu_t *gpu, const char *extensions[], size_t num_extensions, const char *validation_layers[], size_t num_validation_layers, lx_gpu_device_t **device)
{
    LX_ASSERT(gpu, "Invalid gpu");
    LX_ASSERT(device, "Invalid gpu device");

    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_infos[3];
    memset(queue_create_infos, 0, sizeof(VkDeviceQueueCreateInfo) * 3);

    uint32_t num_queue_create_infos = 0;

    if (gpu->graphics_queue_family_index != UINT32_MAX) {
        queue_create_infos[num_queue_create_infos].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[num_queue_create_infos].queueFamilyIndex = gpu->graphics_queue_family_index;
        queue_create_infos[num_queue_create_infos].queueCount = 1;
        queue_create_infos[num_queue_create_infos].pQueuePriorities = &queue_priority;
        ++num_queue_create_infos;
    }

    if (gpu->presentation_queue_family_index != UINT32_MAX && gpu->presentation_queue_family_index != gpu->graphics_queue_family_index) {
        queue_create_infos[num_queue_create_infos].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[num_queue_create_infos].queueFamilyIndex = gpu->presentation_queue_family_index;
        queue_create_infos[num_queue_create_infos].queueCount = 1;
        queue_create_infos[num_queue_create_infos].pQueuePriorities = &queue_priority;
        ++num_queue_create_infos;
    }

    if (gpu->compute_queue_family_index != UINT32_MAX && gpu->compute_queue_family_index != gpu->presentation_queue_family_index) {
        queue_create_infos[num_queue_create_infos].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[num_queue_create_infos].queueFamilyIndex = gpu->compute_queue_family_index;
        queue_create_infos[num_queue_create_infos].queueCount = 1;
        queue_create_infos[num_queue_create_infos].pQueuePriorities = &queue_priority;
        ++num_queue_create_infos;
    }

    VkDeviceCreateInfo create_info = { 0 };
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = queue_create_infos;
    create_info.queueCreateInfoCount = num_queue_create_infos;
    create_info.ppEnabledLayerNames = validation_layers;
    create_info.enabledLayerCount = (uint32_t)num_validation_layers;
    create_info.ppEnabledExtensionNames = extensions;
    create_info.enabledExtensionCount = (uint32_t)num_extensions;

    VkDevice handle;
    VkResult result = vkCreateDevice(gpu->handle, &create_info, NULL, &handle);
    if (result != VK_SUCCESS) {
        LX_LOG_ERROR(LOG_TAG, "Failed to create logical device (Code: %d)", result);
        return LX_ERROR;
    }

    VkQueue compute_queue = VK_NULL_HANDLE;
    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue presentation_queue = VK_NULL_HANDLE;

    if (gpu->compute_queue_family_index != UINT32_MAX) {
        vkGetDeviceQueue(handle, gpu->compute_queue_family_index, 0, &compute_queue);
    }

    if (gpu->graphics_queue_family_index != UINT32_MAX) {
        vkGetDeviceQueue(handle, gpu->graphics_queue_family_index, 0, &graphics_queue);
    }

    if (gpu->presentation_queue_family_index != UINT32_MAX) {
        vkGetDeviceQueue(handle, gpu->presentation_queue_family_index, 0, &presentation_queue);
    }

    *device = lx_alloc(gpu->allocator, sizeof(lx_gpu_device_t));
    **device = (lx_gpu_device_t)
    { 
        .gpu = gpu,
        .shaders = lx_array_create(gpu->allocator, sizeof(lx_shader_t)),
        .handle = handle,
        .compute_queue = compute_queue,
        .graphics_queue = graphics_queue,
        .presentation_queue = presentation_queue
    };

    return LX_SUCCESS;
}

void lx_gpu_destroy_device(lx_gpu_device_t *device)
{
    LX_ASSERT(device, "Invalid device");

    lx_array_for(lx_shader_t, s, device->shaders) {
        vkDestroyShaderModule(device->handle, s->handle, NULL);
    }

    lx_array_destroy(device->shaders);

    vkDestroyDevice(device->handle, NULL);
    *device = (lx_gpu_device_t) { 0 };
}

void lx_gpu_destroy(lx_gpu_t *gpu)
{
    LX_ASSERT(gpu, "Invalid gpu");
    lx_array_destroy(gpu->queue_family_properties);
}

lx_result_t lx_gpu_create_semaphore(lx_gpu_device_t *device, VkSemaphore *semaphore)
{
    VkSemaphoreCreateInfo create_info = { 0 };
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(device->handle, &create_info, NULL, semaphore) != VK_SUCCESS)
        return LX_ERROR;

    return LX_SUCCESS;
}

void lx_gpu_destroy_semaphore(lx_gpu_device_t *device, VkSemaphore semaphore)
{
    vkDestroySemaphore(device->handle, semaphore, NULL);
}

lx_gpu_buffer_t *lx_gpu_create_buffer(lx_gpu_device_t *device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties)
{
    VkBufferCreateInfo create_info = { 0 };
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer buffer;
    if (vkCreateBuffer(device->handle, &create_info, NULL, &buffer) != VK_SUCCESS) {
        LX_LOG_ERROR(LOG_TAG, "Failed to create buffer");
        return NULL;
    }

    VkMemoryRequirements reqs;
    vkGetBufferMemoryRequirements(device->handle, buffer, &reqs);

    VkMemoryAllocateInfo alloc_info = { 0 };
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = reqs.size;
    alloc_info.memoryTypeIndex = find_memory_type_index(device, &reqs, memory_properties);

    VkDeviceMemory memory;
    if (vkAllocateMemory(device->handle, &alloc_info, NULL, &memory) != VK_SUCCESS) {
        LX_LOG_ERROR(LOG_TAG, "Failed to allocate memory");
        return NULL;
    }

    if (vkBindBufferMemory(device->handle, buffer, memory, 0) != VK_SUCCESS) {
        LX_LOG_ERROR(LOG_TAG, "Failed to bind memory to buffer");
        return NULL;
    }

    lx_gpu_buffer_t *gpu_buffer = lx_alloc(device->gpu->allocator, sizeof(lx_gpu_buffer_t));
    *gpu_buffer = (lx_gpu_buffer_t) { .handle = buffer, .memory = memory, .size = size, .offset = 0, .data = NULL };

    return gpu_buffer;
}

void lx_gpu_destroy_buffer(lx_gpu_device_t *device, lx_gpu_buffer_t *buffer)
{
    LX_ASSERT(device, "Invalid device");
    LX_ASSERT(buffer, "Invalid buffer");
    LX_ASSERT(buffer->handle != VK_NULL_HANDLE, "Invalid buffer handle");

    vkDestroyBuffer(device->handle, buffer->handle, NULL);
    vkFreeMemory(device->handle, buffer->memory, NULL);
}

bool lx_gpu_map_memory(lx_gpu_device_t *device, lx_gpu_buffer_t *buffer)
{
    return vkMapMemory(device->handle, buffer->memory, buffer->offset, buffer->size, 0, &buffer->data) == VK_SUCCESS;
}

void lx_gpu_unmap_memory(lx_gpu_device_t *device, lx_gpu_buffer_t *buffer)
{
    vkUnmapMemory(device->handle, buffer->memory);
}

void lx_gpu_buffer_copy_data(lx_gpu_device_t *device, lx_gpu_buffer_t *buffer, const void *data)
{
    LX_ASSERT(lx_gpu_map_memory(device, buffer), "Failed to map gpu memory");
    memcpy(buffer->data, data, buffer->size);
    lx_gpu_unmap_memory(device, buffer);
}

lx_result_t lx_gpu_copy_buffer(lx_gpu_device_t *device, lx_gpu_buffer_t *dst, lx_gpu_buffer_t *src, VkCommandPool command_pool)
{
    LX_ASSERT(device, "Invalid device");
    LX_ASSERT(dst, "Invalid destination buffer");
    LX_ASSERT(dst, "Invalid source buffer");

    VkDeviceSize size = dst->size;

    VkCommandBufferAllocateInfo allocation_info = { 0 };
    allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocation_info.commandPool = command_pool;
    allocation_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    if (vkAllocateCommandBuffers(device->handle, &allocation_info, &command_buffer) != VK_SUCCESS) {
        return LX_ERROR;
    }

    VkCommandBufferBeginInfo begin_info = { 0 };
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
        return LX_ERROR;
    }

    VkBufferCopy region = { 0 };
    region.srcOffset = 0; // Optional
    region.dstOffset = 0; // Optional
    region.size = size;
    vkCmdCopyBuffer(command_buffer, src->handle, dst->handle, 1, &region);

    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = { 0 };
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(device->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(device->graphics_queue);

    return LX_SUCCESS;
}

lx_result_t lx_gpu_create_shader(lx_gpu_device_t *device, const char *code, size_t code_size, uint32_t id, VkShaderStageFlags stage)
{
    LX_ASSERT(device, "Invalid device");
    LX_ASSERT(code && code_size > 0, "Invalid shader byte code");

    LX_ASSERT(!lx_array_exists(device->shaders, shader_id_equals, &id), "Shader already exists");

    lx_shader_t shader = { .handle = 0, .id = id, .stage = stage };

    VkShaderModuleCreateInfo create_info = { 0 };
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pCode = (uint32_t *)code;
    create_info.codeSize = code_size;

    VkResult result = vkCreateShaderModule(device->handle, &create_info, NULL, &shader.handle);
    if (result != VK_SUCCESS) {
        LX_LOG_WARNING(LOG_TAG, "Failed to create shader, id=%d", id);
        return LX_ERROR;
    }

    lx_array_push_back(device->shaders, &shader);
    LX_LOG_DEBUG(LOG_TAG, "Created shader, id=%d", id);

    return LX_SUCCESS;
}

lx_result_t lx_gpu_destroy_shader(lx_gpu_device_t *device, uint32_t id)
{
    LX_ASSERT(device, "Invalid device");

    lx_shader_t *shader = lx_array_find_if(device->shaders, shader_id_equals, &id);
    if (!shader) {
        LX_LOG_ERROR(LOG_TAG, "Shader doesn't exists, id=%d", id);
        return LX_ERROR;
    }

    vkDestroyShaderModule(device->handle, shader->handle, NULL);
    lx_array_remove_if(device->shaders, shader_id_equals, &id);

    return LX_SUCCESS;
}

lx_shader_t *lx_gpu_shader(lx_gpu_device_t *device, uint32_t id)
{
    return lx_array_find_if(device->shaders, shader_id_equals, &id);
}

lx_gpu_image_t *lx_gpu_create_image(lx_gpu_device_t *device, VkExtent2D size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_properties)
{
	LX_ASSERT(device, "Invalid device");
	LX_ASSERT(format != VK_FORMAT_UNDEFINED, "Invalid format");

	VkImageCreateInfo image_create_info = { 0 };
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.extent.width = size.width;
	image_create_info.extent.height = size.height;
	image_create_info.extent.depth = 1;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.format = format;
	image_create_info.tiling = tiling;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage = usage;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkImage handle;
	if (vkCreateImage(device->handle, &image_create_info, NULL, &handle) != VK_SUCCESS) {
		return NULL;
	}

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(device->handle, handle, &memory_requirements);

	VkMemoryAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = memory_requirements.size;
	alloc_info.memoryTypeIndex = find_memory_type_index(device, &memory_requirements, memory_properties);

	VkDeviceMemory memory;
	if (vkAllocateMemory(device->handle, &alloc_info, NULL, &memory) != VK_SUCCESS) {
		vkDestroyImage(device->handle, handle, NULL);
		return NULL;
	}

	if (vkBindImageMemory(device->handle, handle, memory, 0) != VK_SUCCESS) {
		vkDestroyImage(device->handle, handle, NULL);
		vkFreeMemory(device->handle, memory, NULL);
	}

	lx_gpu_image_t *image = lx_alloc(device->gpu->allocator, sizeof(lx_gpu_image_t));
	*image = (lx_gpu_image_t) { .handle = handle, .memory = memory, .offset = 0, .format = format };

	return image;
}

void lx_gpu_destroy_image(lx_gpu_device_t *device, lx_gpu_image_t *image)
{
	LX_ASSERT(device, "Invalid device");
	LX_ASSERT(image, "Invalid image");

	vkDestroyImage(device->handle, image->handle, NULL);
	vkFreeMemory(device->handle, image->memory, NULL);
}
