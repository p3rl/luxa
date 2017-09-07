#include <luxa/fs.h>
#include <windows.h>
#include <stdio.h>

lx_result_t lx_fs_current_directory(lx_string_t *path)
{
	LX_ASSERT(path, "Invalid path");
	
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	size_t i = lx_string_last_index_of(buffer, "\\");
	buffer[i] = '\0';
	
	lx_string_assign_c_str(path, buffer);
	return LX_SUCCESS;
}

lx_result_t lx_fs_read_file(lx_buffer_t *buffer, const char *path)
{
	LX_ASSERT(buffer, "Invalid buffer");
	LX_ASSERT(path, "Invalid path");
	
	FILE *handle;
	errno_t error = fopen_s(&handle, path, "rb");
	if (error != 0)
		return LX_ERROR;

	fseek(handle, 0L, SEEK_END);
	long size = ftell(handle);
	fseek(handle, 0L, SEEK_SET);

	lx_buffer_resize(buffer, (size_t)size);
	fread_s(lx_buffer_data(buffer), (size_t)size, sizeof(char), (size_t)size, handle);
	error = ferror(handle);
	fclose(handle);
	
	if (error) {
		lx_buffer_clear(buffer);
		return LX_ERROR;
	}

	return LX_SUCCESS;
}