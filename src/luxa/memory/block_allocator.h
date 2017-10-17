#pragma once

#include <luxa/platform.h>
#include <luxa/memory/allocator.h>

/*
 * Creates a new instance of a fixed size non thread safe block allocator.
 */
lx_allocator_t *lx_block_allocator_create(lx_allocator_t *allocator, size_t chunk_size, size_t block_size);


/*
 * Destroy block allocator. 
 */
void lx_block_allocator_destroy(lx_allocator_t *block_allocator);
