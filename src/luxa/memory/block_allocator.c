#include <luxa/memory/block_allocator.h>

typedef struct chunk {
	struct chunk *next_chunk;
} chunk_t;

typedef struct block {
	void *buffer;
	struct block *next_block;
} block_t;

typedef struct block_allocator_state {
	lx_allocator_t *allocator;
	size_t chunk_size;
	size_t block_size;
	chunk_t *free_chunks;
	block_t *blocks;
} block_allocator_state_t;

void add_block_to_free_list(block_allocator_state_t *state, block_t *block)
{
	int8_t *p = block->buffer;

	for (size_t i = 0; i < state->block_size; ++i) {
		chunk_t *chunk = (chunk_t *)p;
		chunk->next_chunk = state->free_chunks;
		state->free_chunks = chunk;
		p += state->chunk_size + sizeof(chunk_t);
	}
}

void allocate_block(block_allocator_state_t *state)
{
	block_t *block = lx_alloc(state->allocator, sizeof(block_t));
	block->buffer = lx_alloc(state->allocator, (sizeof(chunk_t) + state->chunk_size) * state->block_size);
	add_block_to_free_list(state, block);
	block->next_block = state->blocks;
	state->blocks = block;
}

static void *block_realloc(lx_allocator_state_t *state, void *p, size_t size)
{
	LX_ASSERT(state, "Invalid state");
	LX_ASSERT(p == NULL, "Block allocator does not support realloc");
	
	block_allocator_state_t *s = (block_allocator_state_t *)state;
	
	if (!size) {
		chunk_t *chunk = p;
		LX_ASSERT(chunk->next_chunk == NULL, "Invalid memory chunk");

		chunk->next_chunk = s->free_chunks;
		s->free_chunks = chunk;
		return NULL;
	}

	LX_ASSERT(size == s->chunk_size, "Requested memory size doesn't match chunk size");

	if (!s->free_chunks) {
		allocate_block(s);
	}

	LX_ASSERT(s->free_chunks, "No free chunks available");

	chunk_t *chunk = s->free_chunks;
	s->free_chunks = chunk->next_chunk;
	chunk->next_chunk = NULL;

	return chunk;
}

lx_allocator_t *lx_block_allocator_create(lx_allocator_t *allocator, size_t chunk_size, size_t block_size)
{
	const size_t num_blocks = 2;
	
	block_allocator_state_t *state = lx_alloc(allocator, sizeof(block_allocator_state_t));
	
	*state = (block_allocator_state_t) {
		.allocator = allocator,
		.chunk_size = chunk_size,
		.block_size = block_size,
		.free_chunks = NULL,
		.blocks = NULL
	};

	for (size_t i = 0; i < num_blocks; ++i) {
		allocate_block(state);
	}

	lx_allocator_t *block_allocator = lx_alloc(allocator, sizeof(lx_allocator_t));
	*block_allocator = (lx_allocator_t) { .state = (lx_allocator_state_t *)state, .realloc = block_realloc };

	return block_allocator;
}

void lx_block_allocator_destroy(lx_allocator_t *block_allocator)
{
	LX_ASSERT(block_allocator, "Invalid block allocator");

	block_allocator_state_t *s = (block_allocator_state_t *)block_allocator->state;
	
	block_t *block = s->blocks;
	while (block) {
		lx_free(s->allocator, block->buffer);
		block_t *tmp = block;
		block = block->next_block;
		lx_free(s->allocator, tmp);
	}
}
