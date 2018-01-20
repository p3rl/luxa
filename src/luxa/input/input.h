#pragma once

#include <luxa/platform.h>
#include <luxa/memory/allocator.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_input lx_input_t;

lx_input_t *lx_input_create(lx_allocator_t *allocator);

void lx_input_destroy(lx_input_t *input);

void lx_input_frame_begin(lx_input_t *input, void *input_message);

void lx_input_frame_end(lx_input_t *input);

#ifdef __cplusplus
}
#endif