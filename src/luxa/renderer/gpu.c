#include <luxa/renderer/gpu.h>
#include <luxa/log.h>

static const char *LOG_TAG = "GPU";

static bool shader_id_equals(lx_shader_t *shader, lx_any_t id)
{
    return shader->id == *(uint32_t*)id;
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

    if (gpu->compute_queue_family_index != UINT32_MAX) {
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

lx_gpu_buffer_t *lx_gpu_create_buffer(lx_gpu_device_t *device, VkDeviceSize size, VkBufferUsageFlags buffer_usage_flags, VkMemoryPropertyFlags memory_property_flags)
{
    return NULL;
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
