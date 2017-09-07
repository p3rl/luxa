#ifndef RENDERER_H
#define RENDERER_H

#include <luxa/memory/allocator.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_renderer lx_renderer_t;

lx_result_t lx_renderer_create(lx_allocator_t *allocator, lx_renderer_t **renderer, void* window_handle, void* module_handle);

void lx_renderer_destroy(lx_allocator_t *allocator, lx_renderer_t *renderer);

#ifdef __cplusplus
}
#endif

#endif // RENDERER_H
