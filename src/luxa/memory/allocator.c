#include <luxa/memory/allocator.h>
#include <stdlib.h>

static void *default_realloc(lx_allocator_state_t *state, void *ptr, size_t size) {
	return realloc(ptr, size);
}

lx_allocator_t* lx_allocator_default() {
	static lx_allocator_t default_allocator = { .state = 0, .realloc = default_realloc };
	return &default_allocator;
}
