#ifndef FS_H
#define FS_H

#include <luxa/collections/string.h>
#include <luxa/collections/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

lx_result_t lx_fs_current_directory(lx_string_t *path);

lx_result_t lx_fs_read_file(lx_buffer_t *buffer, const char *path);

#ifdef __cplusplus
}
#endif

#endif //FS_H