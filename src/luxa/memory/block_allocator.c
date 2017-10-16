#include <luxa/memory/block_allocator.h>

typedef struct chunk {
	void *buffer;
	struct chunk *next_chunk;
} chunk_t;

typedef struct block {
	void *buffer;
	struct block *next_block;
} block_t;

typedef struct block_allocator_state {
	lx_allocator_t *allocator;
	chunk_t *free_chunks;
	block_t *first_block;
} block_allocator_state_t;

static void *block_realloc(lx_allocator_state_t *state, void *p, size_t size)
{
	return NULL;
}

lx_allocator_t *lx_block_allocator_create(lx_allocator_t *allocator, size_t chunk_size, size_t block_size)
{
	const size_t num_blocks = 16;
	
	block_allocator_state_t *state = lx_alloc(allocator, sizeof(block_allocator_state_t));
	
	*state = (block_allocator_state_t) {
		.allocator = allocator,
		.free_chunks = NULL,
		.first_block = NULL
	};

	for (size_t i = 0; i < num_blocks; ++i) {
		block_t *block = lx_alloc(state->allocator, sizeof(block_t));
		block->buffer = lx_alloc(allocator, (sizeof(chunk_t) + chunk_size) * block_size);
		block->next_block = state->first_block;
		state->first_block = block;
	}

	lx_allocator_t *block_allocator = lx_alloc(allocator, sizeof(lx_allocator_t));
	*block_allocator = (lx_allocator_t) { .state = (lx_allocator_state_t *)state, .realloc = block_realloc };

	return block_allocator;
}
