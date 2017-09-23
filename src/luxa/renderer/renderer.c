#include <luxa/renderer/renderer.h>
#include <luxa/renderer/gpu.h>
#include <luxa/renderer/render_pipeline.h>
#include <luxa/renderer/mesh.h>
#include <luxa/log.h>
#include <luxa/collections/array.h>
#include <vulkan/vulkan.h>

#define LOG_TAG "Renderer"

typedef struct render_pipeline {
    VkRenderPass render_pass;
    lx_array_t vertex_shader_ids; //uint32_t;
    lx_array_t fragment_shader_ids; //uint32_t;
    VkViewport viewport;
    VkPipeline handle;
} render_pipeline_t;

typedef struct surface_details {
	VkSurfaceCapabilitiesKHR capabilities;
	lx_array_t *formats;
	lx_array_t *present_modes;
} surface_details_t;

typedef struct command_pool {
	VkCommandPool handle;
	uint32_t queue_family_index;
	lx_array_t *command_buffers; // VkCommandBuffer
} command_pool_t;

typedef struct frame_buffer {
	VkFramebuffer handle;
} frame_buffer_t;

typedef struct swap_chain {
	lx_array_t *images; // VkImage
	lx_array_t *image_views; // VkImageView
	VkSwapchainKHR handle;
	VkPresentModeKHR present_mode;
	VkSurfaceFormatKHR surface_format;
	VkExtent2D extent;
} swap_chain_t;

struct lx_renderer
{
	lx_allocator_t *allocator;
    lx_array_t *gpus; // lx_gpu_t
    lx_gpu_device_t *device;
    lx_array_t *frame_buffers; // frame_buffer_t
    lx_render_pipeline_layout_t *render_pipeline_layout;
    lx_render_pipeline_t *render_pipeline;
	swap_chain_t *swap_chain;
	command_pool_t *command_pool;
	VkInstance instance;
	VkSurfaceKHR presentation_surface;	
	VkRenderPass render_pass;
	VkDebugReportCallbackEXT debug_report_extension;
	VkSemaphore semaphore_image_available;
	VkSemaphore semaphore_render_finished;
	bool record_command_buffer;
};

VkBool32 debug_report_callback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT object_type,
	uint64_t object,
	size_t location,
	int32_t message_code,
	const char* layer_prefix,
	const char* message,
	void* user_data)
{
	LX_LOG_WARNING(LOG_TAG, "%s (%s, %d)", message, layer_prefix, message_code);
	return true;
}

static lx_array_t *get_available_validation_layers(lx_allocator_t *allocator)
{
	uint32_t num_available_layers = 0;
	vkEnumerateInstanceLayerProperties(&num_available_layers, NULL);
	lx_array_t *layer_properties = lx_array_create_with_size(allocator, sizeof(VkLayerProperties), num_available_layers);
	vkEnumerateInstanceLayerProperties(&num_available_layers, lx_array_begin(layer_properties));
	return layer_properties;
}

static lx_array_t *get_available_extensions(lx_allocator_t *allocator)
{
	uint32_t num_extensions = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &num_extensions, NULL);
	if (!num_extensions)
		return lx_array_create(allocator, sizeof(VkExtensionProperties));

	lx_array_t *extension_properties = lx_array_create_with_size(allocator, sizeof(VkExtensionProperties), num_extensions);
	vkEnumerateInstanceExtensionProperties(NULL, &num_extensions, lx_array_begin(extension_properties));
	return extension_properties;
}

static void destroy_surface_details(surface_details_t *details)
{
	lx_array_destroy(details->formats);
	lx_array_destroy(details->present_modes);
	memset(details, 0, sizeof(surface_details_t));
}

static lx_result_t get_surface_details(lx_allocator_t *allocator, lx_gpu_t *gpu, VkSurfaceKHR surface, surface_details_t *details)
{
	memset(details, 0, sizeof(surface_details_t));
	
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu->handle, surface, &details->capabilities) != VK_SUCCESS) {
		memset(details, 0, sizeof(surface_details_t));
		return LX_ERROR;
	}

	uint32_t num_surface_formats;
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(gpu->handle, surface, &num_surface_formats, NULL) != VK_SUCCESS) {
		return LX_ERROR;
	}

	if (num_surface_formats) {
		details->formats = lx_array_create_with_size(allocator, sizeof(VkSurfaceFormatKHR), num_surface_formats);
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(gpu->handle, surface, &num_surface_formats, lx_array_begin(details->formats)) != VK_SUCCESS) {
			lx_array_destroy(details->formats);
			memset(details, 0, sizeof(surface_details_t));
			return LX_ERROR;
		}
	}

	uint32_t num_present_modes;
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(gpu->handle, surface, &num_present_modes, NULL) != VK_SUCCESS) {
		lx_array_destroy(details->formats);
		memset(details, 0, sizeof(surface_details_t));
		return LX_ERROR;
	}

	if (num_present_modes) {
		details->present_modes = lx_array_create_with_size(allocator, sizeof(VkPresentModeKHR), num_present_modes);
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(gpu->handle, surface, &num_present_modes, lx_array_begin(details->present_modes)) != VK_SUCCESS) {
			lx_array_destroy(details->formats);
			lx_array_destroy(details->present_modes);
			memset(details, 0, sizeof(surface_details_t));
			return LX_ERROR;
		}
	}

	return LX_SUCCESS;
}

static lx_result_t create_instance(lx_renderer_t *renderer,
							const char *validation_layers[],
							uint32_t num_valiation_layers,
							const char *extensions[],
							uint32_t num_extensions)
{
	LX_ASSERT(renderer, "Invalid render interface");

	// Log available validaton layers
	lx_array_t *available_validation_layers = get_available_validation_layers(renderer->allocator);
	lx_array_for(VkLayerProperties, p, available_validation_layers) {
		LX_LOG_DEBUG(LOG_TAG, "Found validation layer, name=%s, description=%s", p->layerName, p->description);
	}
	lx_array_destroy(available_validation_layers);

	// Log available extensions
	lx_array_t *available_extensions = get_available_extensions(renderer->allocator);
	if (lx_array_is_empty(available_extensions)) {
		LX_LOG_WARNING(LOG_TAG, "No extensinos found");
	}

	lx_array_for(VkExtensionProperties, p, available_extensions) {
		LX_LOG_DEBUG(LOG_TAG, "Found extension, name=%s", p->extensionName);
	}
	
	lx_array_destroy(available_extensions);

	// Setup application info
	VkApplicationInfo app_info = { 0 };
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Vulkan Lab";
	app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	app_info.pEngineName = "Luxa Engine";
	app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	// Setup create info
	VkInstanceCreateInfo create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;
	create_info.ppEnabledExtensionNames = extensions;
	create_info.enabledExtensionCount = num_extensions;
	create_info.ppEnabledLayerNames = validation_layers;
	create_info.enabledLayerCount = num_valiation_layers;

	// Create Vulkan instance
	VkResult result = vkCreateInstance(&create_info, NULL, &renderer->instance);
	if (result != VK_SUCCESS) {
		if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
			LX_LOG_ERROR(LOG_TAG, "Failed to create Vulkan instance, extension not present");
		}
		else if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
			LX_LOG_ERROR(LOG_TAG, "Incompatible driver");
		}
		else {
			LX_LOG_ERROR(LOG_TAG, "Failed to create Vulkan instance");
		}
		return LX_ERROR;
	}

	return LX_SUCCESS;
}

static lx_result_t create_extensions(lx_renderer_t *renderer)
{
	// Setup debug report extension
	VkDebugReportCallbackCreateInfoEXT debug_report_create_info = { 0 };
	debug_report_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debug_report_create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	debug_report_create_info.pfnCallback = debug_report_callback;

	PFN_vkCreateDebugReportCallbackEXT create_debug_report_extension = NULL;
	*(void **)&create_debug_report_extension = vkGetInstanceProcAddr(renderer->instance, "vkCreateDebugReportCallbackEXT");
	if (create_debug_report_extension) {
		create_debug_report_extension(renderer->instance, &debug_report_create_info, NULL, &renderer->debug_report_extension);
		LX_LOG_DEBUG(LOG_TAG, "Vulkan debug report callback extension [OK]");
	}
	else {
		LX_LOG_WARNING(LOG_TAG, "Vulkan debug report callback extension not present");
	}

	return LX_SUCCESS;
}

static lx_result_t create_surfaces(lx_renderer_t *renderer, void *window_handle, void *module_handle)
{
	VkWin32SurfaceCreateInfoKHR surface_create_info;
	surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_create_info.hwnd = window_handle;
	surface_create_info.hinstance = module_handle;
	surface_create_info.pNext = NULL;
	surface_create_info.flags = 0;

	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = NULL;
	*(void**)&vkCreateWin32SurfaceKHR = vkGetInstanceProcAddr(renderer->instance, "vkCreateWin32SurfaceKHR");
	if (!vkCreateWin32SurfaceKHR) {
		LX_LOG_ERROR(LOG_TAG, "Failed to get function address vkCreateWin32SurfaceKHR when creating surface");
		return LX_ERROR;
	}

	VkResult result = vkCreateWin32SurfaceKHR(renderer->instance, &surface_create_info, NULL, &renderer->presentation_surface);
	if (result != VK_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to create Win32 surface (Error: %d", result);
		return LX_ERROR;
	}

	return LX_SUCCESS;
}

static void destroy_swap_chain(lx_renderer_t *renderer, swap_chain_t *swap_chain)
{
	lx_array_for(VkImageView, iv, swap_chain->image_views) {
		vkDestroyImageView(renderer->device->handle, *iv, NULL);
	}

	vkDestroySwapchainKHR(renderer->device->handle, swap_chain->handle, NULL);

	lx_array_destroy(swap_chain->image_views);
	lx_array_destroy(swap_chain->images);
	lx_free(renderer->allocator, swap_chain);
}

static lx_result_t create_swap_chain(lx_renderer_t *renderer, lx_extent2_t swap_chain_extent, VkSwapchainKHR old_swap_chain_handle)
{
	LX_ASSERT(renderer, "Invalid renderer");
	LX_ASSERT(renderer->device, "Invalid logical device");
	LX_ASSERT(renderer->swap_chain == NULL, "Swap chain already exists");

    lx_gpu_t *gpu = renderer->device->gpu;

	surface_details_t surface_details;
	if (get_surface_details(renderer->allocator, gpu, renderer->presentation_surface, &surface_details) != LX_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to get surface details");
		return LX_ERROR;
	}

	VkSurfaceFormatKHR surface_format = { 0 };
	lx_array_for(VkSurfaceFormatKHR, sf, surface_details.formats) {
		if (sf->format == VK_FORMAT_B8G8R8A8_UNORM && sf->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			surface_format = (VkSurfaceFormatKHR) { .format = VK_FORMAT_B8G8R8A8_UNORM,.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}
	}

	if (surface_format.format == VK_FORMAT_UNDEFINED) {
		surface_format = (VkSurfaceFormatKHR) { .format = VK_FORMAT_B8G8R8A8_UNORM, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}
	
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	lx_array_for(VkPresentModeKHR, pm, surface_details.present_modes) {
		if (*pm == VK_PRESENT_MODE_MAILBOX_KHR) {
			present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		else if (*pm == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			break;
		}
	}

	VkExtent2D extent = surface_details.capabilities.currentExtent;
	if (extent.width == UINT32_MAX) {
		extent = (VkExtent2D) { swap_chain_extent.width, swap_chain_extent.height };
		extent.width = lx_max(surface_details.capabilities.minImageExtent.width, lx_min(surface_details.capabilities.maxImageExtent.width, extent.width));
		extent.height = lx_max(surface_details.capabilities.minImageExtent.height, lx_min(surface_details.capabilities.maxImageExtent.height, extent.height));
	}

	uint32_t image_count = surface_details.capabilities.minImageCount + 1;
	if (surface_details.capabilities.maxImageCount > 0 && image_count > surface_details.capabilities.maxImageCount) {
		image_count = surface_details.capabilities.maxImageCount;
	}
	
	VkSwapchainCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = renderer->presentation_surface;

	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queue_family_indicies[] = { (uint32_t)gpu->graphics_queue_family_index, (uint32_t)gpu->presentation_queue_family_index };

	if (gpu->graphics_queue_family_index != gpu->presentation_queue_family_index) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indicies;
	}
	else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	create_info.preTransform = surface_details.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = old_swap_chain_handle;

	destroy_surface_details(&surface_details);
	
	swap_chain_t *swap_chain = lx_alloc(renderer->allocator, sizeof(swap_chain_t));
	*swap_chain = (swap_chain_t) { .images = 0, .image_views = 0, .present_mode = present_mode, .surface_format = surface_format, .extent = extent };
	
	if (vkCreateSwapchainKHR(renderer->device->handle, &create_info, NULL, &swap_chain->handle) != VK_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to create swap chain");
		return LX_ERROR;
	}

	// Get swap chain images
	uint32_t num_images;
	if (vkGetSwapchainImagesKHR(renderer->device->handle, swap_chain->handle, &num_images, NULL) != VK_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to get number of swap chain image(s)");
		return LX_ERROR;
	}

	swap_chain->images = lx_array_create_with_size(renderer->allocator, sizeof(VkImage), num_images);
	if (vkGetSwapchainImagesKHR(renderer->device->handle, swap_chain->handle, &num_images, lx_array_begin(swap_chain->images)) != VK_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to get swap chain image(s)");
		return LX_ERROR;
	}

	// Create image views for each image
	swap_chain->image_views = lx_array_create_with_size(renderer->allocator, sizeof(VkImageView), num_images);
	VkImageView *image_view = lx_array_begin(swap_chain->image_views);
	lx_array_for(VkImage, image, swap_chain->images) {
		VkImageViewCreateInfo image_view_create_info = { 0 };
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image = *image;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = swap_chain->surface_format.format;
		image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = 1;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = 1;

		if (vkCreateImageView(renderer->device->handle, &image_view_create_info, NULL, image_view) != VK_SUCCESS) {
			LX_LOG_WARNING(LOG_TAG, "Failed to creat swap chain image view");
		}

		++image_view;
	}

	renderer->swap_chain = swap_chain;

	return LX_SUCCESS;
}

static lx_result_t create_render_pass(lx_renderer_t *renderer)
{
	VkAttachmentDescription color_attachment = { 0 };
	color_attachment.format = renderer->swap_chain->surface_format.format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = { 0 };
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = { 0 };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkRenderPassCreateInfo render_pass_create_info = { 0 };
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = 1;
	render_pass_create_info.pAttachments = &color_attachment;
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass;

	VkSubpassDependency dependency = { 0 };
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	render_pass_create_info.dependencyCount = 1;
	render_pass_create_info.pDependencies = &dependency;

	if (vkCreateRenderPass(renderer->device->handle, &render_pass_create_info, NULL, &renderer->render_pass) != VK_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to create render pass");
		return LX_ERROR;
	}

	return LX_SUCCESS;
}

static void destroy_frame_buffers(lx_renderer_t *renderer)
{
	if (!renderer->frame_buffers)
		return;

	lx_array_for(frame_buffer_t, fb, renderer->frame_buffers) {
		vkDestroyFramebuffer(renderer->device->handle, fb->handle, NULL);
	}
	
	lx_array_destroy(renderer->frame_buffers);
	renderer->frame_buffers = NULL;
}

static lx_result_t create_frame_buffers(lx_renderer_t *renderer)
{
	LX_ASSERT(renderer->render_pass, "Invalid render pass");
	
	destroy_frame_buffers(renderer);
	renderer->frame_buffers = lx_array_create(renderer->allocator, sizeof(frame_buffer_t));

	lx_array_for(VkImageView, iv, renderer->swap_chain->image_views) {
		VkImageView attachments[] = { *iv };

		VkFramebufferCreateInfo frame_buffer_create_info = { 0 };
		frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_create_info.renderPass = renderer->render_pass;
		frame_buffer_create_info.attachmentCount = 1;
		frame_buffer_create_info.pAttachments = attachments;
		frame_buffer_create_info.width = renderer->swap_chain->extent.width;
		frame_buffer_create_info.height = renderer->swap_chain->extent.height;
		frame_buffer_create_info.layers = 1;

		frame_buffer_t fb = { 0 };
		if (vkCreateFramebuffer(renderer->device->handle, &frame_buffer_create_info, NULL, &fb.handle) != VK_SUCCESS) {
			LX_LOG_ERROR(LOG_TAG, "Failed to create frame buffer");
			return LX_ERROR;
		}

		lx_array_push_back(renderer->frame_buffers, &fb);
	}

	return LX_SUCCESS;
}

static void destroy_command_pool_buffers(lx_renderer_t *renderer, command_pool_t *command_pool)
{
	LX_ASSERT(command_pool, "Invalid command pool");

	if (!command_pool->command_buffers)
		return;
	
	uint32_t num_buffers = (uint32_t)lx_array_size(command_pool->command_buffers);
	VkCommandBuffer *buffers = lx_array_begin(command_pool->command_buffers);

	vkFreeCommandBuffers(renderer->device->handle, command_pool->handle, num_buffers, buffers);
	lx_array_destroy(command_pool->command_buffers);
	command_pool->command_buffers = NULL;
}

static lx_result_t create_command_pool_buffers(lx_renderer_t *renderer, command_pool_t *command_pool, size_t num_buffers)
{
	LX_ASSERT(renderer->command_pool->command_buffers == NULL, "Command pool buffers already exists");
	
	VkCommandBufferAllocateInfo cmd_buffer_alloc_info = { 0 };
	cmd_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_buffer_alloc_info.commandPool = command_pool->handle;
	cmd_buffer_alloc_info.commandBufferCount = (uint32_t)num_buffers;
	cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	lx_array_t *command_buffers = lx_array_create_with_size(renderer->allocator, sizeof(VkCommandBuffer), num_buffers);
	
	if (vkAllocateCommandBuffers(renderer->device->handle, &cmd_buffer_alloc_info, lx_array_begin(command_buffers)) != VK_SUCCESS) {
		lx_array_destroy(command_buffers);
		return LX_ERROR;
	}

	renderer->command_pool->command_buffers = command_buffers;

	return LX_SUCCESS;
}

static lx_result_t create_command_pool(lx_renderer_t *renderer, uint32_t queue_family_index)
{
	LX_ASSERT(renderer->command_pool == NULL, "Command pool already exists");

	command_pool_t *command_pool = lx_alloc(renderer->allocator, sizeof(command_pool_t));
	
	*command_pool = (command_pool_t)
	{ 
		.handle = 0,
		.queue_family_index = queue_family_index,
		.command_buffers = 0
	};

	VkCommandPoolCreateInfo create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.queueFamilyIndex = queue_family_index;
	create_info.flags = 0;

	if (vkCreateCommandPool(renderer->device->handle, &create_info, NULL, &command_pool->handle) != VK_SUCCESS) {
		lx_free(renderer->allocator, command_pool);
		return LX_ERROR;
	}

	renderer->command_pool = command_pool;

	return LX_SUCCESS;
}

static void destroy_command_pool(lx_renderer_t *renderer)
{
	if (!renderer->command_pool)
		return;
	
	vkDestroyCommandPool(renderer->device->handle, renderer->command_pool->handle, NULL);
	lx_array_destroy(renderer->command_pool->command_buffers);
	*renderer->command_pool = (command_pool_t) { 0 };
}

lx_result_t lx_renderer_create(lx_allocator_t *allocator, lx_renderer_t **renderer, void* window_handle, lx_extent2_t window_size, void* module_handle)
{
	LX_ASSERT(allocator, "Invalid allocator");
	LX_ASSERT(renderer, "Invalid renderer");

	const char *layer_names[] = { "VK_LAYER_LUNARG_standard_validation" };
	const uint32_t num_layer_names = sizeof(layer_names) / sizeof(char*);

	const char *extension_names[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME };
	const uint32_t num_extension_names = sizeof(extension_names) / sizeof(char*);

    lx_renderer_t *vulkan_renderer = lx_alloc(allocator, sizeof(lx_renderer_t));
	*vulkan_renderer = (lx_renderer_t) { 0 };
	vulkan_renderer->allocator = allocator;
	vulkan_renderer->record_command_buffer = true;

	// Initialize Vulkan instance
	lx_result_t result = create_instance(vulkan_renderer, layer_names, num_layer_names, extension_names, num_extension_names);
	if (result != LX_SUCCESS) {
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG(LOG_TAG, "Vulkan instance [OK]");

	// Initialize extension(s)
	result = create_extensions(vulkan_renderer);
	if (result != LX_SUCCESS) {
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG(LOG_TAG, "Vulkan extensions [OK]");

	// Create surface(s)
	result = create_surfaces(vulkan_renderer, window_handle, module_handle);
	if (LX_FAILED(result)) {
		LX_LOG_ERROR(LOG_TAG, "Failed to initialize surface(s)");
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG(LOG_TAG, "Surface(s) [OK]");

	// Initialize gpu(s)
    if (lx_gpu_all_available(vulkan_renderer->allocator, vulkan_renderer->instance, vulkan_renderer->presentation_surface, &vulkan_renderer->gpus) != LX_SUCCESS) {
        LX_LOG_ERROR(LOG_TAG, "No gpu(s) found");
        lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
        return LX_ERROR;
    }

	// Create gpu device(s)
	const char *device_extension_names[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const uint32_t num_device_extension_names = sizeof(device_extension_names) / sizeof(char*);
    
    lx_gpu_t *gpu = lx_array_at(vulkan_renderer->gpus, 0);
    LX_LOG_DEBUG(LOG_TAG, "Using %s as main gpu", gpu->properties.deviceName);
    
    if (lx_gpu_create_device(gpu, device_extension_names, num_device_extension_names, layer_names, num_layer_names, &vulkan_renderer->device) != LX_SUCCESS) {
        LX_LOG_ERROR(LOG_TAG, "Failed to create gpu device");
        lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
        return LX_ERROR;
    }
	LX_LOG_DEBUG(LOG_TAG, "Graphics device(s) [OK]");

	// Create swap chain
	result = create_swap_chain(vulkan_renderer, window_size, VK_NULL_HANDLE);
	if (result != LX_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to initialize swap chain");
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG(LOG_TAG, "Swap chain [OK]");

	if (create_render_pass(vulkan_renderer) != LX_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to creat render pass");
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return LX_ERROR;
	}
	LX_LOG_DEBUG(LOG_TAG, "Render pass [OK]");
	
	if (create_frame_buffers(vulkan_renderer) != LX_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed create to frame buffers");
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return LX_ERROR;
	}
	LX_LOG_DEBUG(LOG_TAG, "Frame buffer(s) [OK]");

	if (create_command_pool(vulkan_renderer, vulkan_renderer->device->gpu->graphics_queue_family_index) != LX_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to create command pool");
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return LX_ERROR;
	}
	LX_LOG_DEBUG(LOG_TAG, "Command pool [OK]");

	if (create_command_pool_buffers(vulkan_renderer, vulkan_renderer->command_pool, lx_array_size(vulkan_renderer->frame_buffers)) != LX_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to create command pool buffers");
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return LX_ERROR;
	}
	LX_LOG_DEBUG(LOG_TAG, "Command pool buffers [OK]");
	 
	// Create semaphore(s)
	if (lx_gpu_create_semaphore(vulkan_renderer->device, &vulkan_renderer->semaphore_image_available) != LX_SUCCESS ||
        lx_gpu_create_semaphore(vulkan_renderer->device, &vulkan_renderer->semaphore_render_finished) != LX_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to create semaphore(s)");
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return LX_ERROR;
	}

	*renderer = (lx_renderer_t*)vulkan_renderer;
	return LX_SUCCESS;
}

void lx_renderer_destroy(lx_allocator_t *allocator, lx_renderer_t *renderer)
{
	LX_ASSERT(allocator, "Invalid allocator");
	LX_ASSERT(renderer, "Invalid renderer");

	// Destroy semaphores
	lx_gpu_destroy_semaphore(renderer->device, renderer->semaphore_image_available);
    lx_gpu_destroy_semaphore(renderer->device, renderer->semaphore_render_finished);
	
	// Destroy command pool
	destroy_command_pool(renderer);
	
	// Destroy frame buffer(s)
	destroy_frame_buffers(renderer);
	
	// Destroy render pipline(s)
    if (renderer->render_pipeline_layout)
        lx_render_pipeline_destroy_layout(renderer->device, renderer->render_pipeline_layout);

    if (renderer->render_pipeline)
        lx_render_pipeline_destroy(renderer->device, renderer->render_pipeline);
	
	// Destroy render passe(s)
	if (renderer->render_pass) {
		vkDestroyRenderPass(renderer->device->handle, renderer->render_pass, NULL);
	}
	
	// Destroy swap chain
	if (renderer->swap_chain) {
		
		// Destroy image view(s)
		if (renderer->swap_chain->image_views) {
			lx_array_for(VkImageView, image_view, renderer->swap_chain->image_views) {
				vkDestroyImageView(renderer->device->handle, *image_view, NULL);
			}
			lx_array_destroy(renderer->swap_chain->image_views);
			lx_array_destroy(renderer->swap_chain->images);
            renderer->swap_chain->image_views = NULL;
            renderer->swap_chain->images = NULL;
		}

		vkDestroySwapchainKHR(renderer->device->handle, renderer->swap_chain->handle, NULL);
		lx_free(renderer->allocator, renderer->swap_chain);
        renderer->swap_chain = NULL;
	}
	
	// Destroy gpu devices
	if (renderer->device) {
        lx_gpu_destroy_device(renderer->device);
        renderer->device = NULL;
	}
	
	// Destroy gpu(s)
	if (renderer->gpus) {
        lx_array_for(lx_gpu_t, gpu, renderer->gpus) {
            lx_gpu_destroy(gpu);
        }
        renderer->gpus = NULL;
	}

	// Destroy surface(s)
	if (renderer->presentation_surface) {
		vkDestroySurfaceKHR(renderer->instance, renderer->presentation_surface, NULL);
	}

	// Destroy extensions
	PFN_vkDestroyDebugReportCallbackEXT destroy_debug_report_extension = NULL;
	*(void **)&destroy_debug_report_extension = vkGetInstanceProcAddr(renderer->instance, "vkDestroyDebugReportCallbackEXT");
	
	if (destroy_debug_report_extension && renderer->debug_report_extension) {
		destroy_debug_report_extension(renderer->instance, renderer->debug_report_extension, NULL);
	}
	
	// Destroy Vulkan instance
	if (renderer->instance) {
		vkDestroyInstance(renderer->instance, NULL);
	}
	
	lx_free(allocator, renderer);
}

lx_result_t lx_renderer_create_shader(lx_renderer_t *renderer, lx_buffer_t *code, uint32_t id, lx_shader_stage_t stage)
{
	LX_ASSERT(code && code->size > 0, "Invalid shader byte code");
    return lx_gpu_create_shader(renderer->device, lx_buffer_data(code), lx_buffer_size(code), id, stage);
}

lx_result_t lx_renderer_create_render_pipeline(lx_renderer_t *renderer, uint32_t vertex_shader_id, uint32_t fragment_shader_id)
{
	LX_ASSERT(renderer, "Invalid renderer");
    LX_ASSERT(!renderer->render_pipeline, "Render pipeline already exists");

    renderer->render_pipeline_layout = lx_render_pipeline_create_layout(renderer->allocator);
    
    lx_render_pipeline_add_shader(renderer->render_pipeline_layout, vertex_shader_id);
    lx_render_pipeline_add_shader(renderer->render_pipeline_layout, fragment_shader_id);
    lx_render_pipeline_set_viewport_extent(renderer->render_pipeline_layout, renderer->swap_chain->extent);
    lx_render_pipeline_set_scissor_extent(renderer->render_pipeline_layout, renderer->swap_chain->extent);

    VkVertexInputBindingDescription mesh_vertex_binding = { 0 };
    mesh_vertex_binding.binding = 0;
    mesh_vertex_binding.stride = sizeof(lx_vec3_t);
    mesh_vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription mesh_vertex_attribute = { 0 };
    mesh_vertex_attribute.binding = 0;
    mesh_vertex_attribute.location = 0;
    mesh_vertex_attribute.offset = 0;
    mesh_vertex_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;

    lx_render_pipeline_add_vertex_binding(renderer->render_pipeline_layout, &mesh_vertex_binding);
    lx_render_pipeline_add_vertex_attribute(renderer->render_pipeline_layout, &mesh_vertex_attribute);

    if (lx_render_pipeline_create(renderer->device, renderer->render_pipeline_layout, renderer->render_pass, &renderer->render_pipeline) != LX_SUCCESS) {
        LX_LOG_ERROR(LOG_TAG, "Failed to create render pipeline");
        return LX_ERROR;
    }

	return LX_SUCCESS;
}

void lx_renderer_render_frame(lx_renderer_t *renderer, lx_scene_t *scene)
{
	if (renderer->record_command_buffer) {
        
        // Record command buffers
		for (uint32_t i = 0; i < lx_array_size(renderer->command_pool->command_buffers); ++i) {
			VkCommandBufferBeginInfo buffer_begin_info = { 0 };
			buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			buffer_begin_info.pInheritanceInfo = NULL; // Optional

			VkCommandBuffer *cb = lx_array_at(renderer->command_pool->command_buffers, i);
			vkBeginCommandBuffer(*cb, &buffer_begin_info);

			frame_buffer_t *fb = lx_array_at(renderer->frame_buffers, i);
			VkRenderPassBeginInfo render_pass_begin_info = { 0 };
			render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.renderPass = renderer->render_pass;
			render_pass_begin_info.framebuffer = fb->handle;

			render_pass_begin_info.renderArea.offset = (VkOffset2D) { 0, 0 };
			render_pass_begin_info.renderArea.extent = renderer->swap_chain->extent;

			VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
			render_pass_begin_info.clearValueCount = 1;
			render_pass_begin_info.pClearValues = &clear_color;

			vkCmdBeginRenderPass(*cb, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(*cb, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->render_pipeline->handle);


            // Draw meshes
            const size_t scene_size = lx_scene_size(scene);
            for (lx_scene_node_t node = 1; node < scene_size; ++node) {
                lx_renderable_t renderable = lx_scene_renderable(scene, node);

                if (!lx_is_some_renderable(renderable))
                    continue;

                lx_scene_render_data_t *rd = lx_scene_render_data(scene, renderable);
                if (!rd)
                    continue;

                lx_mesh_t *mesh = rd->data;
                lx_gpu_buffer_t *vertex_buffer = lx_mesh_vertex_buffer(mesh);
                lx_gpu_buffer_t *index_buffer = lx_mesh_index_buffer(mesh);

                VkBuffer buffers[] = { vertex_buffer->handle };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(*cb, 0, 1, buffers, offsets);
                vkCmdBindIndexBuffer(*cb, index_buffer->handle, 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(*cb, (uint32_t)lx_mesh_num_indices(mesh), 1, 0, 0, 0);
            }

			vkCmdEndRenderPass(*cb);

			if (vkEndCommandBuffer(*cb) != VK_SUCCESS) {
				LX_LOG_ERROR(LOG_TAG, "Failed to record command buffer");
				return;
			}
		}

		renderer->record_command_buffer = false;
	}

	vkQueueWaitIdle(renderer->device->presentation_queue);
	
	// Acquire image
	uint32_t image_index;
	VkResult result = vkAcquireNextImageKHR(renderer->device->handle, renderer->swap_chain->handle, INTMAX_MAX, renderer->semaphore_image_available, VK_NULL_HANDLE, &image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		//lx_renderer_reset_swap_chain(renderer, 1, 2);
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		LX_LOG_ERROR(LOG_TAG, "Failed to acquire swap chain image!");
		return;
	}


	VkSubmitInfo submit_info = { 0 };
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { renderer->semaphore_image_available };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;

	VkCommandBuffer *cmd_buffer = lx_array_at(renderer->command_pool->command_buffers, image_index);
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = cmd_buffer;

	VkSemaphore signals[] = { renderer->semaphore_render_finished };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signals;

	result = vkQueueSubmit(renderer->device->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	if (result != VK_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to sumbit draw command buffer (Error: %d)", result);
		return;
	}

	// Presentation	
	VkPresentInfoKHR present_info = { 0 };
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signals;

	VkSwapchainKHR swap_chains[] = { renderer->swap_chain->handle };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
	present_info.pImageIndices = &image_index;

	present_info.pResults = NULL; // Optional

	if (vkQueuePresentKHR(renderer->device->presentation_queue, &present_info) != VK_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to present image");
	}

	vkQueueWaitIdle(renderer->device->presentation_queue);
}

void lx_renderer_device_wait_idle(lx_renderer_t *renderer)
{
	LX_ASSERT(renderer, "Invalid renderer");
	
	LX_ASSERT(renderer->device, "Invalid device");
	vkDeviceWaitIdle(renderer->device->handle);
}

lx_result_t lx_renderer_reset_swap_chain(lx_renderer_t *renderer, lx_extent2_t swap_chain_extent)
{
	LX_ASSERT(renderer, "Invalid renderer");

	vkQueueWaitIdle(renderer->device->presentation_queue);

	LX_LOG_DEBUG(LOG_TAG, "Resetting swap chain");

	destroy_frame_buffers(renderer);
	destroy_command_pool_buffers(renderer, renderer->command_pool);

    lx_render_pipeline_destroy(renderer->device, renderer->render_pipeline);
    renderer->render_pipeline = NULL;
	
	vkDestroyRenderPass(renderer->device->handle, renderer->render_pass, NULL);
	renderer->render_pass = VK_NULL_HANDLE;

	swap_chain_t* old_swap_chain = renderer->swap_chain;
	renderer->swap_chain = NULL;

	VkResult result = create_swap_chain(renderer, swap_chain_extent, old_swap_chain->handle);
	if (result != LX_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to reset swap chain");
		return LX_ERROR;
	}

    lx_render_pipeline_set_viewport_extent(renderer->render_pipeline_layout, renderer->swap_chain->extent);
    lx_render_pipeline_set_scissor_extent(renderer->render_pipeline_layout, renderer->swap_chain->extent);

	destroy_swap_chain(renderer, old_swap_chain);

	if (create_render_pass(renderer) != LX_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to creat render pass");
		return LX_ERROR;
	}
	LX_LOG_DEBUG(LOG_TAG, "Render pass [OK]");

	if (create_frame_buffers(renderer) != LX_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to frame buffers");
		return LX_ERROR;
	}
	LX_LOG_DEBUG(LOG_TAG, "Frame buffer(s) [OK]");

	if (create_command_pool_buffers(renderer, renderer->command_pool, lx_array_size(renderer->frame_buffers)) != LX_SUCCESS) {
		LX_LOG_ERROR(LOG_TAG, "Failed to create command pool buffers");
		return LX_ERROR;
	}
	LX_LOG_DEBUG(LOG_TAG, "Command pool buffers [OK]");

	renderer->record_command_buffer = true;
    return lx_render_pipeline_create(renderer->device, renderer->render_pipeline_layout, renderer->render_pass, &renderer->render_pipeline);
}

void lx_renderer_initialize_scene(lx_renderer_t *renderer, lx_scene_t *scene)
{
    const size_t scene_size = lx_scene_size(scene);
    for (size_t i = 1; i < scene_size; ++i) {
        lx_renderable_t renderable = lx_scene_renderable(scene, i);
        
        if (!lx_is_some_renderable(renderable))
            continue;

        lx_scene_render_data_t *rd = lx_scene_render_data(scene, renderable);
        if (!rd)
            continue;

        lx_mesh_t *mesh = rd->data;
        size_t size = lx_mesh_vertices_byte_size(mesh);
        
        // Create vertex buffer
        lx_gpu_buffer_t *staging_buffer = lx_gpu_create_buffer(renderer->device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        LX_ASSERT(staging_buffer, "Failed to create buffer");
                
        lx_gpu_map_memory(renderer->device, staging_buffer);
        memcpy(staging_buffer->data, lx_mesh_vertices(mesh), lx_mesh_vertices_byte_size(mesh));
        lx_gpu_unmap_memory(renderer->device, staging_buffer);

        lx_gpu_buffer_t *vertex_buffer = lx_gpu_create_buffer(renderer->device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);

        lx_gpu_copy_buffer(renderer->device, vertex_buffer, staging_buffer, renderer->command_pool->handle);
        lx_gpu_destroy_buffer(renderer->device, staging_buffer);

        // Create index buffer
        size = lx_mesh_indices_byte_size(mesh);
        staging_buffer = lx_gpu_create_buffer(renderer->device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        LX_ASSERT(staging_buffer, "Failed to create buffer");

        lx_gpu_map_memory(renderer->device, staging_buffer);
        memcpy(staging_buffer->data, lx_mesh_indices(mesh), size);
        lx_gpu_unmap_memory(renderer->device, staging_buffer);

        lx_gpu_buffer_t *index_buffer = lx_gpu_create_buffer(renderer->device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);

        lx_gpu_copy_buffer(renderer->device, index_buffer, staging_buffer, renderer->command_pool->handle);
        lx_gpu_destroy_buffer(renderer->device, staging_buffer);

        lx_mesh_set_vertex_buffer(mesh, vertex_buffer);
        lx_mesh_set_index_buffer(mesh, index_buffer);
    }
}