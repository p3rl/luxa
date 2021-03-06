#pragma once

#include <luxa/platform.h>
#include <luxa/memory/allocator.h>
#include <luxa/collections/array.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_gpu {
    lx_allocator_t *allocator;
    VkPhysicalDevice handle;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory_properties;
    uint32_t compute_queue_family_index;
    uint32_t graphics_queue_family_index;
    uint32_t presentation_queue_family_index;
    lx_array_t *queue_family_properties; // VkQueueFamilyProperties
} lx_gpu_t;

typedef struct lx_shader {
    VkShaderModule handle;
    uint32_t id;
    VkShaderStageFlags stage;
} lx_shader_t;

typedef struct lx_gpu_device {
    lx_gpu_t *gpu;
    lx_array_t *shaders;
    VkDevice handle;
    VkQueue graphics_queue;
    VkQueue compute_queue;
    VkQueue presentation_queue;
} lx_gpu_device_t;

typedef struct lx_gpu_buffer {
    VkBuffer handle;
    VkDeviceMemory memory;
    VkDeviceSize size;
    VkDeviceSize offset;
    void *data;
} lx_gpu_buffer_t;

typedef struct lx_gpu_image {
	VkImage handle;
	VkDeviceMemory memory;
	VkDeviceSize offset;
	VkFormat format;
} lx_gpu_image_t;

lx_result_t lx_gpu_all_available(lx_allocator_t *allocator, VkInstance instance, VkSurfaceKHR presentation_surface, lx_array_t **gpus);

lx_result_t lx_gpu_create_device(lx_gpu_t *gpu, const char *extensions[], size_t num_extensions, const char *validation_layers[], size_t num_validation_layers, lx_gpu_device_t **device);

void lx_gpu_destroy_device(lx_gpu_device_t *device);

void lx_gpu_destroy(lx_gpu_t *gpu);

lx_result_t lx_gpu_create_semaphore(lx_gpu_device_t *device, VkSemaphore *semaphore);

void lx_gpu_destroy_semaphore(lx_gpu_device_t *device, VkSemaphore semaphore);

lx_gpu_buffer_t *lx_gpu_create_buffer(lx_gpu_device_t *device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties);

void lx_gpu_destroy_buffer(lx_gpu_device_t *device, lx_gpu_buffer_t *buffer);

bool lx_gpu_map_memory(lx_gpu_device_t *device, lx_gpu_buffer_t *buffer);

void lx_gpu_unmap_memory(lx_gpu_device_t *device, lx_gpu_buffer_t *buffer);

void lx_gpu_buffer_copy_data(lx_gpu_device_t *device, lx_gpu_buffer_t *buffer, const void *data);

lx_result_t lx_gpu_copy_buffer(lx_gpu_device_t *device, lx_gpu_buffer_t *dst, lx_gpu_buffer_t *src, VkCommandPool command_pool);

lx_result_t lx_gpu_create_shader(lx_gpu_device_t *device, const char *code, size_t code_size, uint32_t id, VkShaderStageFlags stage);

lx_result_t lx_gpu_destroy_shader(lx_gpu_device_t *device, uint32_t id);

lx_shader_t *lx_gpu_shader(lx_gpu_device_t *device, uint32_t id);

lx_gpu_image_t *lx_gpu_create_image(lx_gpu_device_t *device, VkExtent2D size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

void lx_gpu_destroy_image(lx_gpu_device_t *device, lx_gpu_image_t *image);

#ifdef __cplusplus
}
#endif
