#ifndef FS_H
#define FS_H

#include <luxa/collections/string.h>

#ifdef __cplusplus
extern "C" {
#endif

lx_string_t *lx_fs_current_directory(lx_string_t *path);

#ifdef __cplusplus
}
#endif

#endif //FS_H