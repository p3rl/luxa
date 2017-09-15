#include <luxa/collections/map.h>

struct lx_map
{
	lx_allocator_t *allocator;
	size_t element_size;
	void *buffer;
	size_t size;
	size_t capacity;
	uint64_t *keys;
	lx_any_t *items;
};

lx_map_t *lx_map_create(lx_allocator_t *allocator, size_t element_size)
{
	LX_ASSERT(allocator, "Invalid allocator");
	LX_ASSERT(element_size, "Invalid map element size");

	lx_map_t *map = lx_alloc(allocator, sizeof(lx_map_t));
	*map = (lx_map_t) { .allocator = allocator, .element_size = element_size };

	return map;
}

void lx_map_destroy(lx_map_t *map)
{
	LX_ASSERT(map, "Invalid map");
	
	lx_free(map->allocator, map);
	*map = (lx_map_t) { 0 };
}

void lx_map_insert(lx_map_t *map, uint64_t key, lx_any_t item)
{
	LX_ASSERT(map, "Invalid map");
}

lx_any_t lx_map_get(lx_map_t *map, uint64_t key)
{
	LX_ASSERT(map, "Invalid map");

	return NULL;
}

lx_any_t lx_map_get_or_default(lx_map_t *map, uint64_t key, lx_any_t default_value)
{
	LX_ASSERT(map, "Invalid map");

	return NULL;
}