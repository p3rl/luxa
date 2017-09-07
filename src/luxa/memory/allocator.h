#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <luxa/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

// Allocator state
typedef struct lx_allocator_state lx_allocator_state_t;

// Allocator interface
typedef struct lx_allocator {
	lx_allocator_state_t *state;
	void *(*realloc)(lx_allocator_state_t *state, void *ptr, size_t size);
} lx_allocator_t;

lx_allocator_t* lx_allocator_default();

static inline void *lx_alloc(lx_allocator_t *allocator, size_t size) {
	return allocator->realloc(allocator->state, NULL, size);
}

static inline void *lx_realloc(lx_allocator_t *allocator, void *ptr, size_t size) {
	return allocator->realloc(allocator->state, ptr, size);
}

static inline void lx_free(lx_allocator_t *allocator, void *ptr) {
	allocator->realloc(allocator->state, ptr, 0);
}

#ifdef __cplusplus
}
#endif

#endif // ALLOCATOR_H
