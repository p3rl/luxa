#include <luxa/renderer/renderer.h>
#include <luxa/collections/array.h>
#include <luxa/log.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <string.h>

typedef struct swap_chain {
	lx_array_t *images;
	lx_array_t *image_views;
	VkSwapchainKHR handle;
	VkPresentModeKHR present_mode;
	VkSurfaceFormatKHR surface_format;
	VkExtent2D extent;
} swap_chain_t;

typedef struct physical_device
{
	VkPhysicalDevice handle;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memory_properties;
	VkSurfaceCapabilitiesKHR surface_capabilities;
	lx_array_t *queue_family_properties;  // VkQueueFamilyProperties
	lx_array_t *surface_formats;
	lx_array_t *present_modes;
	int graphics_queue_family_index;
	int compute_queue_family_index;
	int presentation_queue_family_index;
} physical_device_t;

typedef struct logical_device
{
	VkDevice handle;
	VkQueue graphics_queue;
	VkQueue compute_queue;
	VkQueue presentation_queue;
} logical_device_t;

typedef struct vulkan_renderer
{
	lx_allocator_t *allocator;
	lx_array_t *physical_devices; // physical_device_t
	physical_device_t* physical_device;
	logical_device_t* device;
	swap_chain_t *swap_chain;
	VkInstance instance;
	VkDebugReportCallbackEXT debug_report_extension;
	VkSurfaceKHR presentation_surface;
} vulkan_renderer_t;

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
	LX_LOG_WARNING("Renderer", "%s (%s, %d)", message, layer_prefix, message_code);
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

static lx_array_t *get_physical_devices(vulkan_renderer_t *renderer)
{
	uint32_t num_devices = 0;
	vkEnumeratePhysicalDevices(renderer->instance, &num_devices, NULL);
	if (!num_devices)
		return NULL;

	VkPhysicalDevice *available_physical_devices = lx_alloc(renderer->allocator, sizeof(VkPhysicalDevice) * num_devices);
	vkEnumeratePhysicalDevices(renderer->instance, &num_devices, available_physical_devices);

	renderer->physical_devices = lx_array_create(renderer->allocator, sizeof(physical_device_t));

	for (uint32_t i = 0; i < num_devices; ++i) {
		physical_device_t physical_device = { 0 };
		physical_device.graphics_queue_family_index = -1;
		physical_device.compute_queue_family_index = -1;
		physical_device.presentation_queue_family_index = -1;

		physical_device.handle = available_physical_devices[i];

		// Device properties
		vkGetPhysicalDeviceProperties(physical_device.handle, &physical_device.properties);

		// Features
		vkGetPhysicalDeviceFeatures(physical_device.handle, &physical_device.features);

		// Memory properties
		vkGetPhysicalDeviceMemoryProperties(physical_device.handle, &physical_device.memory_properties);

		// Queue family properties
		uint32_t num_queue_family_properties = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device.handle, &num_queue_family_properties, NULL);

		if (num_queue_family_properties) {
			physical_device.queue_family_properties = lx_array_create_with_size(renderer->allocator, sizeof(VkQueueFamilyProperties), num_queue_family_properties);
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device.handle, &num_queue_family_properties, lx_array_begin(physical_device.queue_family_properties));

			// Setup queue family indices
			for (uint32_t j = 0; j < lx_array_size(physical_device.queue_family_properties); ++j) {
				VkQueueFamilyProperties *queue_properties = lx_array_at(physical_device.queue_family_properties, j);
				if (queue_properties->queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					physical_device.graphics_queue_family_index = j;
				}
				else if (queue_properties->queueFlags & VK_QUEUE_COMPUTE_BIT) {
					physical_device.compute_queue_family_index = j;
				}

				VkBool32 present_support = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physical_device.handle, j, renderer->presentation_surface, &present_support);
				if (present_support) {
					physical_device.presentation_queue_family_index = j;
				}
			}
		}

		// Surface capabilities
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device.handle, renderer->presentation_surface, &physical_device.surface_capabilities) != VK_SUCCESS) {
			LX_LOG_WARNING("Renderer", "Physical device has no surface capabilities");
		}

		// Surface formats
		uint32_t num_surface_formats;
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device.handle, renderer->presentation_surface, &num_surface_formats, NULL) != VK_SUCCESS) {
			LX_LOG_WARNING("Renderer", "Failed to get number of surface format(s)");
		}

		if (num_surface_formats) {
			physical_device.surface_formats = lx_array_create_with_size(renderer->allocator, sizeof(VkSurfaceFormatKHR), num_surface_formats);

			if (vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device.handle, renderer->presentation_surface, &num_surface_formats, lx_array_begin(physical_device.surface_formats)) != VK_SUCCESS) {
				LX_LOG_WARNING("Renderer", "Failed to get surface format(s)");
			}
		}

		// Presentaton modes
		uint32_t num_presentation_modes;
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device.handle, renderer->presentation_surface, &num_presentation_modes, NULL) != VK_SUCCESS) {
			LX_LOG_WARNING("Renderer", "Failed to get number of presentation modes");
		}

		if (num_presentation_modes) {
			physical_device.present_modes = lx_array_create_with_size(renderer->allocator, sizeof(VkPresentModeKHR), num_presentation_modes);
			
			if (vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device.handle, renderer->presentation_surface, &num_presentation_modes, lx_array_begin(physical_device.present_modes)) != VK_SUCCESS) {
				LX_LOG_WARNING("Renderer", "Failed to get presentation mode(s)");
			}
		}

		lx_array_push_back(renderer->physical_devices, &physical_device);
	}

	lx_free(renderer->allocator, available_physical_devices);
	
	return renderer->physical_devices;
}

static lx_result_t create_instance(vulkan_renderer_t *renderer,
							const char *validation_layers[],
							uint32_t num_valiation_layers,
							const char *extensions[],
							uint32_t num_extensions)
{
	LX_ASSERT(renderer, "Invalid render interface");

	// Log available validaton layers
	lx_array_t *available_validation_layers = get_available_validation_layers(renderer->allocator);
	lx_array_for_each(VkLayerProperties, p, available_validation_layers) {
		LX_LOG_DEBUG("Renderer", "Found validation layer, name=%s, description=%s", p->layerName, p->description);
	}
	lx_array_destroy(available_validation_layers);

	// Log available extensions
	lx_array_t *available_extensions = get_available_extensions(renderer->allocator);
	if (lx_array_is_empty(available_extensions)) {
		LX_LOG_WARNING("Renderer", "No extensinos found");
	}

	lx_array_for_each(VkExtensionProperties, p, available_extensions) {
		LX_LOG_DEBUG("Renderer", "Found extension, name=%s", p->extensionName);
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
			LX_LOG_ERROR("Renderer", "Failed to create Vulkan instance, extension not present");
		}
		else if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
			LX_LOG_ERROR("Renderer", "Incompatible driver");
		}
		else {
			LX_LOG_ERROR("Renderer", "Failed to create Vulkan instance");
		}
		return LX_ERROR;
	}

	return LX_SUCCESS;
}

static lx_result_t initialize_extensions(vulkan_renderer_t *renderer)
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
		LX_LOG_DEBUG("Renderer", "Vulkan debug report callback extension [OK]");
	}
	else {
		LX_LOG_WARNING("Renderer", "Vulkan debug report callback extension not present");
	}

	return LX_SUCCESS;
}

static lx_result_t initialize_surfaces(vulkan_renderer_t *renderer, void *window_handle, void *module_handle)
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
		LX_LOG_ERROR("Renderer", "Failed to get function address vkCreateWin32SurfaceKHR when creating surface");
		return LX_ERROR;
	}

	VkResult result = vkCreateWin32SurfaceKHR(renderer->instance, &surface_create_info, NULL, &renderer->presentation_surface);
	if (result != VK_SUCCESS) {
		LX_LOG_ERROR("Renderer", "Failed to create Win32 surface (Error: %d", result);
		return LX_ERROR;
	}

	return LX_SUCCESS;
}

static lx_result_t initialize_physical_devices(vulkan_renderer_t *renderer)
{
	lx_array_t *physical_devices = get_physical_devices(renderer);
	if (!physical_devices) {
		LX_LOG_ERROR("Renderer", "No devices found");
		return LX_ERROR;
	}

	// Log physical devices
	lx_array_for_each(physical_device_t, p, physical_devices) {
		LX_LOG_DEBUG("Renderer", "Found device, name=%s", p->properties.deviceName);
	}

	renderer->physical_device = lx_array_begin(renderer->physical_devices);
	LX_LOG_DEBUG("Renderer", "Using %s as main physical device", renderer->physical_device->properties.deviceName);
	
	return LX_SUCCESS;
}

static lx_result_t initialize_logial_devices(
	vulkan_renderer_t *renderer,
	const char *extension_names[],
	size_t num_extension_names,
	const char *layer_names[],
	size_t num_layer_names)
{
	LX_ASSERT(renderer, "Invalid renderer");
	LX_ASSERT(renderer->physical_device, "No physical device(s)");

	// TODO: Find the best GPU
	physical_device_t *physical_device = renderer->physical_device;
	
	VkPhysicalDeviceFeatures device_features = { 0 };

	float queue_priority = 1.0f;
	
	VkDeviceQueueCreateInfo queue_create_infos[2];
	memset(queue_create_infos, 0, sizeof(VkDeviceQueueCreateInfo) * 2);
	
	queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_infos[0].queueFamilyIndex = physical_device->graphics_queue_family_index;
	queue_create_infos[0].queueCount = 1;
	queue_create_infos[0].pQueuePriorities = &queue_priority;
	
	queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_infos[1].queueFamilyIndex = physical_device->presentation_queue_family_index;
	queue_create_infos[1].queueCount = 1;
	queue_create_infos[1].pQueuePriorities = &queue_priority;

	VkDeviceCreateInfo create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos = queue_create_infos;
	create_info.queueCreateInfoCount = physical_device->presentation_queue_family_index != physical_device->graphics_queue_family_index ? 2 : 1;
	create_info.ppEnabledLayerNames = layer_names;
	create_info.enabledLayerCount = (uint32_t)num_layer_names;
	create_info.ppEnabledExtensionNames = extension_names;
	create_info.enabledExtensionCount = (uint32_t)num_extension_names;
	
	VkDevice device;
	VkResult result = vkCreateDevice(physical_device->handle, &create_info, NULL, &device);
	if (result != VK_SUCCESS) {
		LX_LOG_ERROR("Renderer", "Failed to create logical device (Code: %d)", result);
		return LX_ERROR;
	}

	renderer->device = lx_alloc(renderer->allocator, sizeof(logical_device_t));
	renderer->device->handle = device;

	vkGetDeviceQueue(device, physical_device->graphics_queue_family_index, 0, &renderer->device->graphics_queue);
	vkGetDeviceQueue(device, physical_device->presentation_queue_family_index, 0, &renderer->device->presentation_queue);
	
	return LX_SUCCESS;
}

static lx_result_t initialize_swap_chain(vulkan_renderer_t *renderer)
{
	LX_ASSERT(renderer, "Invalid renderer");
	LX_ASSERT(renderer->physical_device, "No physical device(s)");

	physical_device_t *physical_device = renderer->physical_device;
	
	VkSurfaceFormatKHR surface_format = { 0 };
	lx_array_for_each(VkSurfaceFormatKHR, sf, physical_device->surface_formats) {
		if (sf->format == VK_FORMAT_B8G8R8A8_UNORM && sf->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			surface_format = (VkSurfaceFormatKHR) { .format = VK_FORMAT_B8G8R8A8_UNORM,.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}
	}

	if (surface_format.format == VK_FORMAT_UNDEFINED) {
		surface_format = (VkSurfaceFormatKHR) { .format = VK_FORMAT_B8G8R8A8_UNORM, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}
	
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	lx_array_for_each(VkPresentModeKHR, pm, physical_device->present_modes) {
		if (*pm == VK_PRESENT_MODE_MAILBOX_KHR) {
			present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		else if (*pm == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			break;
		}
	}

	VkExtent2D extent = physical_device->surface_capabilities.currentExtent;

	uint32_t image_count = physical_device->surface_capabilities.minImageCount + 1;
	if (physical_device->surface_capabilities.maxImageCount > 0 && image_count > physical_device->surface_capabilities.maxImageCount) {
		image_count = physical_device->surface_capabilities.maxImageCount;
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

	uint32_t queue_family_indicies[] = { (uint32_t)physical_device->graphics_queue_family_index, (uint32_t)physical_device->presentation_queue_family_index };

	if (physical_device->graphics_queue_family_index != physical_device->presentation_queue_family_index) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indicies;
	}
	else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	create_info.preTransform = physical_device->surface_capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;
	
	swap_chain_t *swap_chain = lx_alloc(renderer->allocator, sizeof(swap_chain_t));
	*swap_chain = (swap_chain_t) { .images = 0, .image_views = 0, .present_mode = present_mode, .surface_format = surface_format, .extent = extent };
	
	if (vkCreateSwapchainKHR(renderer->device->handle, &create_info, NULL, &swap_chain->handle) != VK_SUCCESS) {
		LX_LOG_ERROR("Renderer", "Failed to create swap chain");
		return LX_ERROR;
	}

	// Get swap chain images
	uint32_t num_images;
	if (vkGetSwapchainImagesKHR(renderer->device->handle, swap_chain->handle, &num_images, NULL) != VK_SUCCESS) {
		LX_LOG_ERROR("Renderer", "Failed to get number of swap chain image(s)");
		return LX_ERROR;
	}

	swap_chain->images = lx_array_create_with_size(renderer->allocator, sizeof(VkImage), num_images);
	if (vkGetSwapchainImagesKHR(renderer->device->handle, swap_chain->handle, &num_images, lx_array_begin(swap_chain->images)) != VK_SUCCESS) {
		LX_LOG_ERROR("Renderer", "Failed to get swap chain image(s)");
		return LX_ERROR;
	}

	// Create image views for each image
	swap_chain->image_views = lx_array_create_with_size(renderer->allocator, sizeof(VkImageView), num_images);
	VkImageView *image_view = lx_array_begin(swap_chain->image_views);
	lx_array_for_each(VkImage, image, swap_chain->images) {
		VkImageViewCreateInfo create_info = { 0 };
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image = *image;
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = swap_chain->surface_format.format;
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;

		if (vkCreateImageView(renderer->device->handle, &create_info, NULL, image_view) != VK_SUCCESS) {
			LX_LOG_WARNING("Renderer", "Failed to creat swap chain image view");
		}

		++image_view;
	}

	renderer->swap_chain = swap_chain;

	return LX_SUCCESS;
}

lx_result_t lx_renderer_create(lx_allocator_t *allocator, lx_renderer_t **renderer, void* window_handle, void* module_handle)
{
	LX_ASSERT(allocator, "Invalid allocator");
	LX_ASSERT(renderer, "Invalid renderer");

	const char *layer_names[] = { "VK_LAYER_LUNARG_standard_validation" };
	const uint32_t num_layer_names = sizeof(layer_names) / sizeof(char*);

	const char *extension_names[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME };
	const uint32_t num_extension_names = sizeof(extension_names) / sizeof(char*);

	vulkan_renderer_t *vulkan_renderer = lx_alloc(allocator, sizeof(vulkan_renderer_t));
	memset(vulkan_renderer, 0, sizeof(vulkan_renderer_t));
	vulkan_renderer->allocator = allocator;

	// Initialize Vulkan instance
	lx_result_t result = create_instance(vulkan_renderer, layer_names, num_layer_names, extension_names, num_extension_names);
	if (result != LX_SUCCESS) {
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG("Renderer", "Vulkan instance [OK]");

	// Initialize extension(s)
	result = initialize_extensions(vulkan_renderer);
	if (result != LX_SUCCESS) {
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG("Renderer", "Vulkan extensions [OK]");

	// Initialize surface(s)
	result = initialize_surfaces(vulkan_renderer, window_handle, module_handle);
	if (LX_FAILED(result)) {
		LX_LOG_ERROR("Renderer", "Failed to initialize surface(s)");
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG("Renderer", "Surface(s) [OK]");

	// Initialize physical device(s)
	result = initialize_physical_devices(vulkan_renderer);
	if (LX_FAILED(result)) {
		LX_LOG_ERROR("Renderer", "Failed to initialize physical device(s)");
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG("Renderer", "Physical device(s) [OK]");

	// Initialize logical device(s)
	const char *device_extension_names[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const uint32_t num_device_extension_names = sizeof(device_extension_names) / sizeof(char*);

	result = initialize_logial_devices(vulkan_renderer, device_extension_names, num_device_extension_names, layer_names, num_layer_names);
	if (result != LX_SUCCESS) {
		LX_LOG_ERROR("Renderer", "Failed to initialize logical device(s)");
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG("Renderer", "Logical device(s) [OK]");

	// Initialize swap chain
	result = initialize_swap_chain(vulkan_renderer);
	if (result != LX_SUCCESS) {
		LX_LOG_ERROR("Renderer", "Failed to initialize swap chain");
		lx_renderer_destroy(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG("Renderer", "Swap chain [OK]");
	 
	// Initialize image views
	*renderer = (lx_renderer_t*)vulkan_renderer;
	return LX_SUCCESS;
}

void lx_renderer_destroy(lx_allocator_t *allocator, lx_renderer_t *renderer)
{
	LX_ASSERT(allocator, "Invalid allocator");
	LX_ASSERT(renderer, "Invalid renderer");

	vulkan_renderer_t *vulkan_renderer = (vulkan_renderer_t*)renderer;

	// Destroy swap chain
	if (vulkan_renderer->swap_chain) {
		vkDestroySwapchainKHR(vulkan_renderer->device->handle, vulkan_renderer->swap_chain->handle, NULL);
		lx_array_destroy(vulkan_renderer->swap_chain->images);
		lx_array_destroy(vulkan_renderer->swap_chain->image_views);
		lx_free(vulkan_renderer->allocator, vulkan_renderer->swap_chain);
		vulkan_renderer->swap_chain = NULL;
	}
	
	// Destroy logical devices
	if (vulkan_renderer->device) {
		vkDestroyDevice(vulkan_renderer->device->handle, NULL);
		lx_free(vulkan_renderer->allocator, vulkan_renderer->device);
		vulkan_renderer->device = NULL;
	}
	
	// Destroy physical devices
	if (vulkan_renderer->physical_devices) {
		lx_array_for_each(physical_device_t, physical_device, vulkan_renderer->physical_devices) {
			lx_array_destroy(physical_device->queue_family_properties);
		}
		lx_array_destroy(vulkan_renderer->physical_devices);
		vulkan_renderer->physical_devices = NULL;
	}

	// Destroy surface(s)
	if (vulkan_renderer->presentation_surface) {
		vkDestroySurfaceKHR(vulkan_renderer->instance, vulkan_renderer->presentation_surface, NULL);
	}

	// Destroy extensions
	PFN_vkDestroyDebugReportCallbackEXT destroy_debug_report_extension = NULL;
	*(void **)&destroy_debug_report_extension = vkGetInstanceProcAddr(vulkan_renderer->instance, "vkDestroyDebugReportCallbackEXT");
	
	if (destroy_debug_report_extension && vulkan_renderer->debug_report_extension) {
		destroy_debug_report_extension(vulkan_renderer->instance, vulkan_renderer->debug_report_extension, NULL);
	}
	
	// Destroy Vulkan instance
	if (vulkan_renderer->instance) {
		vkDestroyInstance(vulkan_renderer->instance, NULL);
	}
	
	lx_free(allocator, vulkan_renderer);
}
