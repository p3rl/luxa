#include <luxa/fs.h>
#include <windows.h>

lx_string_t *lx_fs_current_directory(lx_string_t *path)
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	size_t i = lx_string_last_index_of(buffer, "\\");
	buffer[i] = '\0';
	
	return path ? lx_string_assign_c_str(path, buffer) : lx_string_from_c_str(NULL, buffer);
}