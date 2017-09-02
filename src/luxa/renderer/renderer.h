#ifndef RENDERER_H
#define RENDERER_H

#include <luxa/memory/allocator.h>

typedef struct lx_renderer lx_renderer_t;

#ifdef __cplusplus
extern "C" {
#endif

lx_result_t lx_create_renderer(lx_allocator_t *allocator, lx_renderer_t **renderer, void* window_handle, void* module_handle);

void lx_destroy_renderer(lx_allocator_t *allocator, lx_renderer_t *renderer);

#ifdef __cplusplus
}
#endif

#endif // RENDERER_H
