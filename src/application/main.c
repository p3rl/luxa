#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <luxa/memory/allocator.h>
#include <luxa/collections/array.h>
#include <luxa/collections/string.h>
#include <luxa/renderer/renderer.h>
#include <luxa/log.h>
#include <luxa/fs.h>
#include <luxa/renderer/scene.h>
#include <luxa/renderer/mesh.h>
#include <luxa/renderer/camera.h>

lx_renderer_t *renderer = NULL;
lx_camera_t *camera = NULL;

void debug_log(time_t time, lx_log_level_t log_level, const char* tag, const char *message, void *user_data)
{
	struct tm tm_info;
	localtime_s(&tm_info, &time);
	char time_string[64];
	strftime(time_string, 64, "%H:%M:%S", &tm_info);

	char log[2048];
	sprintf_s(log, 2048, "[%s][%s][%s]: %s\n", time_string, lx_log_level_to_c_str(log_level), tag, message);

	printf(log);
	OutputDebugString(log);
}

LRESULT CALLBACK handle_window_message(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_PAINT:
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_SIZE: {
			lx_renderer_reset_swap_chain(renderer, (lx_extent2_t) { LOWORD(lParam), HIWORD(lParam) });
		}
		break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
	}

	return 0;
}

int WinMain(HINSTANCE instance_handle, HINSTANCE prev_instance_handle, LPSTR cmd_line, int cmd_show)
{
	lx_allocator_t *allocator = lx_allocator_default();
	lx_initialize_log(allocator, LX_LOG_LEVEL_TRACE);
	lx_register_log_target(LX_LOG_LEVEL_TRACE, debug_log, NULL);

	const char *window_class_name = "LuxaApp";

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = handle_window_message;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = instance_handle;
	wcex.hIcon = LoadIcon(instance_handle, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = window_class_name;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex)) {
		LX_LOG_ERROR(NULL, "Failed register window class");
		return 1;
	}

	lx_extent2_t window_size = { 800, 600 };
	HWND window_handle = CreateWindow(
		window_class_name,
		"Luxa Engine 0.1.0",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		window_size.width, window_size.height,
		NULL,
		NULL,
		instance_handle,
		NULL
	);

	if (!window_handle) {
		LX_LOG_ERROR(NULL, "Failed to create main window");
		return 1;
	}

	lx_renderer_create(allocator, &renderer, window_handle, window_size, instance_handle);

	lx_buffer_t *shader_buffer = lx_buffer_create_empty(NULL);
	lx_fs_read_file(shader_buffer, "C:\\git\\luxa\\build\\bin\\Debug\\vert.spv");
	lx_renderer_create_shader(renderer, shader_buffer, 1, LX_SHADER_STAGE_VERTEX);

	lx_fs_read_file(shader_buffer, "C:\\git\\luxa\\build\\bin\\Debug\\frag.spv");
	lx_renderer_create_shader(renderer, shader_buffer, 2, LX_SHADER_STAGE_FRAGMENT);

	lx_renderer_create_render_pipeline(renderer, 1, 2);

    lx_mesh_t *mesh = lx_mesh_create(allocator);
    
    const float size = 1.0f;

	lx_vertex_t vertices[] =
	{
		// Front
		{ { 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, -1.0f },{ 1.0f, 0.0f, 0.0f } }, // 0
		{ { 0.0f, size, 0.0f },{ 0.0f, 0.0f, -1.0f },{ 1.0f, 0.0f, 0.0f } }, // 1
		{ { size, size, 0.0f },{ 0.0f, 0.0f, -1.0f },{ 1.0f, 0.0f, 0.0f } }, // 2
		{ { size, 0.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{ 1.0f, 0.0f, 0.0f } }, // 3

		// Right
		{ { size, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } }, // 4
		{ { size, size, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } }, // 5
		{ { size, size, size },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } }, // 6
		{ { size, 0.0f, size },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } }, // 7

		// Back
		{ { size, 0.0f, size },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } }, // 8   
		{ { size, size, size },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } }, // 9
		{ { 0.0f, size, size },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } }, // 10
		{ { 0.0f, 0.0f, size },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } }, // 11

		// Left
		{ { 0.0f, 0.0f, size },{ -1.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 0.0f } }, // 12
		{ { 0.0f, size, size },{ -1.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 0.0f } }, // 13
		{ { 0.0f, size, 0.0f },{ -1.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 0.0f } }, // 14
		{ { 0.0f, 0.0f, 0.0f },{ -1.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 0.0f } }, // 15
		
		// Top
		{ { 0.0f, size, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f, 1.0f } }, // 16
		{ { 0.0f, size, size },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f, 1.0f } }, // 17
		{ { size, size, size },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f, 1.0f } }, // 18
		{ { size, size, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f, 1.0f } }, // 19

		// Bottom
		{ { size, 0.0f, size },{ 0.0f, -1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f } }, // 20
		{ { size, 0.0f, 0.0f },{ 0.0f, -1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f } }, // 21
		{ { 0.0f, 0.0f, 0.0f },{ 0.0f, -1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f } }, // 22
		{ { 0.0f, 0.0f, size },{ 0.0f, -1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f } }  // 23
	};

    uint32_t indices[] =
    {
        // Front
        0, 1, 2,
        0, 2, 3,

		//// Right
		4, 5, 6,
		4, 6, 7,

		//// Back
		8, 9, 10,
		8, 10, 11,

		//// Left
		12, 13, 14,
		12, 14, 15,

		//// Top
		16, 17, 18,
		16, 18, 19,

		//// Bottom
		20, 21, 22,
		20, 22, 23,
	};

    lx_mesh_set_vertices(mesh, vertices, sizeof(vertices)/ sizeof(vertices[0]));
    lx_mesh_set_indices(mesh, indices, sizeof(indices) / sizeof(indices[0]));

    lx_scene_t *scene = lx_scene_create(allocator);
    lx_scene_node_t node = lx_scene_create_node(scene, lx_scene_root_node());
    lx_renderable_t renderable = lx_scene_create_renderable(scene, LX_RENDERABLE_TYPE_MESH, mesh);
    lx_scene_attach_renderable(scene, node, renderable);

    lx_renderer_initialize_scene(renderer, scene);

    camera = lx_camera_create(allocator);
    lx_camera_set_projection(camera, 0.1f, 1000.0f, lx_radians(45.0f));

    lx_vec3_t camera_pos = { 5.0f, 5.0f, -5.0f };
    lx_vec3_t camera_target = { 0.0f, 0.0f, 0.0f };
    lx_vec3_t camera_up = { 0, 1.0f, 0 };
    lx_camera_look_at(camera, &camera_target, &camera_pos, &camera_up);

	ShowWindow(window_handle, cmd_show);
	UpdateWindow(window_handle);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		lx_renderer_render_frame(renderer, scene, camera);
		lx_renderer_device_wait_idle(renderer);
	}

	LX_LOG_INFO(NULL, "Shutting down...");
		
	lx_renderer_destroy(allocator, renderer);
	lx_shutdown_log();
	
	return (int)msg.wParam;
}
