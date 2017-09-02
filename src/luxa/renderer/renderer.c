#include <luxa/renderer/renderer.h>
#include <luxa/collections/array.h>
#include <luxa/log.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <string.h>

typedef struct physical_device
{
	VkPhysicalDevice handle;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memory_properties;
	lx_array_t *queue_family_properties;  // VkQueueFamilyProperties
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
	logical_device_t* device;
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

lx_array_t *get_available_validation_layers(lx_allocator_t *allocator)
{
	uint32_t num_available_layers = 0;
	vkEnumerateInstanceLayerProperties(&num_available_layers, NULL);
	lx_array_t *layer_properties = lx_array_create_with_size(allocator, sizeof(VkLayerProperties), num_available_layers);
	vkEnumerateInstanceLayerProperties(&num_available_layers, lx_array_begin(layer_properties));
	return layer_properties;
}

lx_array_t *get_available_extensions(lx_allocator_t *allocator)
{
	uint32_t num_extensions = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &num_extensions, NULL);
	if (!num_extensions)
		return lx_array_create(allocator, sizeof(VkExtensionProperties));

	lx_array_t *extension_properties = lx_array_create_with_size(allocator, sizeof(VkExtensionProperties), num_extensions);
	vkEnumerateInstanceExtensionProperties(NULL, &num_extensions, lx_array_begin(extension_properties));
	return extension_properties;
}

lx_array_t *get_physical_devices(vulkan_renderer_t *renderer)
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

		lx_array_push_back(renderer->physical_devices, &physical_device);
	}

	lx_free(renderer->allocator, available_physical_devices);
	
	return renderer->physical_devices;
}

lx_result_t create_instance(vulkan_renderer_t *renderer,
							const char *validation_layers[],
							uint32_t num_valiation_layers,
							const char *extensions[],
							uint32_t num_extensions)
{
	LX_ASSERT(renderer, "Invalid render interface");

	// Log available validaton layers
	lx_array_t *available_validation_layers = get_available_validation_layers(renderer->allocator);
	for (uint32_t i = 0; i < lx_array_size(available_validation_layers); ++i) {
		VkLayerProperties *properties = lx_array_at(available_validation_layers, i);
		LX_LOG_DEBUG("Renderer", "Found validation layer, name=%s, description=%s", properties->layerName, properties->description);
	}
	lx_array_destroy(available_validation_layers);

	// Log available extensions
	lx_array_t *available_extensions = get_available_extensions(renderer->allocator);
	if (lx_array_is_empty(available_extensions)) {
		LX_LOG_WARNING("Renderer", "No extensinos found");
	}

	for (uint32_t i = 0; i < lx_array_size(available_extensions); ++i) {
		VkExtensionProperties *properties = lx_array_at(available_extensions, i);
		LX_LOG_DEBUG("Renderer", "Found extension, name=%s", properties->extensionName);
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

lx_result_t initialize_extensions(vulkan_renderer_t *renderer)
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

lx_result_t initialize_surfaces(vulkan_renderer_t *renderer, void *window_handle, void *module_handle)
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

lx_result_t initialize_physical_devices(vulkan_renderer_t *renderer)
{
	lx_array_t *physical_devices = get_physical_devices(renderer);
	if (!physical_devices) {
		LX_LOG_ERROR("Renderer", "No devices found");
		return LX_ERROR;
	}

	// Log physical devices
	for (uint32_t i = 0; i < lx_array_size(physical_devices); ++i) {
		physical_device_t *physical_devce = lx_array_at(physical_devices, i);
		LX_LOG_DEBUG("Renderer", "Found device, name=%s", physical_devce->properties.deviceName);
	}
	
	return LX_SUCCESS;
}

lx_result_t initialize_logial_devices(vulkan_renderer_t *renderer)
{
	LX_ASSERT(renderer, "Invalid renderer");
	LX_ASSERT(lx_array_size(renderer->physical_devices), "No physical device(s)");

	// TODO: Find the best GPU
	physical_device_t *physical_device = lx_array_begin(renderer->physical_devices);
	
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

lx_result_t lx_create_renderer(lx_allocator_t *allocator, lx_renderer_t **renderer, void* window_handle, void* module_handle)
{
	LX_ASSERT(allocator, "Invalid allocator");
	LX_ASSERT(renderer, "Invalid renderer");

	const char *validaton_layers[] = { "VK_LAYER_LUNARG_standard_validation" };
	const uint32_t num_validation_layers = sizeof(validaton_layers) / sizeof(char*);

	const char *extensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report" };
	const uint32_t num_extensions = sizeof(extensions) / sizeof(char*);

	vulkan_renderer_t *vulkan_renderer = lx_alloc(allocator, sizeof(vulkan_renderer_t));
	memset(vulkan_renderer, 0, sizeof(vulkan_renderer_t));
	vulkan_renderer->allocator = allocator;

	// Initialize Vulkan instance
	lx_result_t result = create_instance(vulkan_renderer, validaton_layers, num_validation_layers, extensions, num_extensions);
	if (result != LX_SUCCESS) {
		lx_destroy_renderer(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG("Renderer", "Vulkan instance [OK]");

	// Initialize extension(s)
	result = initialize_extensions(vulkan_renderer);
	if (result != LX_SUCCESS) {
		lx_destroy_renderer(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG("Renderer", "Vulkan extensions [OK]");

	// Initialize surface(s)
	result = initialize_surfaces(vulkan_renderer, window_handle, module_handle);
	if (LX_FAILED(result)) {
		LX_LOG_ERROR("Renderer", "Failed to initialize surface(s)");
		lx_destroy_renderer(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG("Renderer", "Surface(s) [OK]");

	// Initialize physical device(s)
	result = initialize_physical_devices(vulkan_renderer);
	if (LX_FAILED(result)) {
		LX_LOG_ERROR("Renderer", "Failed to initialize physical device(s)");
		lx_destroy_renderer(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG("Renderer", "Physical device(s) [OK]");

	// Initialize logical device(s)
	result = initialize_logial_devices(vulkan_renderer);
	if (result != LX_SUCCESS) {
		LX_LOG_ERROR("Renderer", "Failed to initialize logical device(s)");
		lx_destroy_renderer(allocator, (lx_renderer_t*)vulkan_renderer);
		return result;
	}
	LX_LOG_DEBUG("Renderer", "Logical device(s) [OK]");

	// Initialize swap chain

	// Initialize image views

	*renderer = (lx_renderer_t*)vulkan_renderer;
	return LX_SUCCESS;
}

void lx_destroy_renderer(lx_allocator_t *allocator, lx_renderer_t *renderer)
{
	LX_ASSERT(allocator, "Invalid allocator");
	LX_ASSERT(renderer, "Invalid renderer");

	vulkan_renderer_t *vulkan_renderer = (vulkan_renderer_t*)renderer;

	// Destroy logical devices
	if (vulkan_renderer->device) {
		vkDestroyDevice(vulkan_renderer->device->handle, NULL);
		lx_free(vulkan_renderer->allocator, vulkan_renderer->device);
	}
	
	// Destroy physical devices
	if (vulkan_renderer->physical_devices) {
		for (uint32_t i = 0; i < lx_array_size(vulkan_renderer->physical_devices); ++i) {
			physical_device_t *physical_device = lx_array_at(vulkan_renderer->physical_devices, i);
			lx_array_destroy(physical_device->queue_family_properties);
		}
		lx_array_destroy(vulkan_renderer->physical_devices);
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
