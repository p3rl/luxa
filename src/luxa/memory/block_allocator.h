#pragma once

#include <luxa/platform.h>
#include <luxa/memory/allocator.h>

lx_allocator_t *lx_block_allocator_create(lx_allocator_t *allocator, size_t chunk_size, size_t block_size);
