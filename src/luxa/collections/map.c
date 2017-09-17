#include <luxa/collections/map.h>
#include <string.h>

#define MAP_DEFAULT_CAPACITY 16
#define MAP_LOAD_FACTOR 0.5
#define MAP_PROBE_COUNT 8

struct lx_map
{
	lx_allocator_t *allocator;
	size_t element_size;
	lx_hash_func_t hash;
	
	void *buffer;
	size_t size;
	size_t capacity;
	
	size_t *keys;
	uint8_t *entry_states;
	lx_any_t *items;
};

static void rehash(lx_map_t *map);

static bool find_empty_bucket(const lx_map_t *map, size_t hash, size_t *index)
{
	size_t bucket = hash % map->capacity;
	if (!map->entry_states[bucket]) {
		*index = bucket;
		return true;
	}

	for (size_t j = 1; j < MAP_PROBE_COUNT; ++j) {
		bucket = (bucket + j) % map->capacity;
		if (!map->entry_states[bucket]) {
			*index = bucket;
			return true;
		}
	}

	return false;
}

static bool find_bucket(const lx_map_t *map, size_t hash, size_t *index)
{
	size_t bucket = hash % map->capacity;
	if (map->keys[bucket] == hash) {
		*index = bucket;
		return true;
	}

	for (size_t i = 1; i < MAP_PROBE_COUNT; ++i) {
		bucket = (bucket + i) % map->capacity;
		if (map->keys[bucket] == hash) {
			*index = bucket;
			return true;
		}
	}

	return false;
}

static void insert(lx_map_t *map, size_t hash, lx_any_t item)
{
	size_t index = 0;
	bool found = find_empty_bucket(map, hash, &index);
	while (!found) {
		rehash(map);
		found = find_empty_bucket(map, hash, &index);
	}
	
	map->keys[index] = hash;
	map->entry_states[index] = 1;
	memcpy(((char *)map->items) + map->element_size * index, item, map->element_size);
	map->size++;
}

static void rehash(lx_map_t *map)
{
	const size_t bytes = sizeof(size_t) + sizeof(uint8_t) + map->element_size;
	
	lx_map_t old_map = *map;

	const size_t new_capacity = map->capacity * 2;
	
	map->buffer = lx_alloc(map->allocator, bytes * new_capacity);
	map->capacity = new_capacity;
	map->size = 0;

	memset(map->buffer, 0, bytes * new_capacity);
	
	map->keys = (size_t *)map->buffer;
	map->entry_states = (uint8_t *)(map->keys + map->capacity);
	map->items = (lx_any_t *)(map->entry_states + map->capacity);

	for (size_t i = 0; i < old_map.capacity; ++i) {
		if (!old_map.entry_states[i])
			continue;
		
		size_t hash = old_map.keys[i];
		lx_any_t item = ((char *)old_map.items) + old_map.element_size * i;
		insert(map, hash, item);
	}

	lx_free(map->allocator, old_map.buffer);
}

lx_map_t *lx_map_create(lx_allocator_t *allocator, size_t element_size, lx_hash_func_t hash)
{
	LX_ASSERT(allocator, "Invalid allocator");
	LX_ASSERT(element_size, "Invalid map element size");

	const size_t bytes = sizeof(size_t) + sizeof(uint8_t) + element_size;

	lx_map_t *map = lx_alloc(allocator, sizeof(lx_map_t));
	
	*map = (lx_map_t)
	{ 
		.allocator = allocator,
		.element_size = element_size,
		.hash = hash,
		.buffer = lx_alloc(allocator, bytes * MAP_DEFAULT_CAPACITY),
		.capacity = MAP_DEFAULT_CAPACITY,
		.size = 0
	};

	memset(map->buffer, 0, bytes * MAP_DEFAULT_CAPACITY);

	map->keys = (size_t *)map->buffer;
	map->entry_states = (uint8_t *)(map->keys + map->capacity);
	map->items = (lx_any_t *)(map->entry_states + map->capacity);

	return map;
}

void lx_map_destroy(lx_map_t *map)
{
	LX_ASSERT(map, "Invalid map");
	
	lx_free(map->allocator, map->buffer);
	lx_free(map->allocator, map);
	*map = (lx_map_t) { 0 };
}

void lx_map_insert(lx_map_t *map, lx_any_t key, lx_any_t item)
{
	LX_ASSERT(map, "Invalid map");

	double load_factor = ((double)map->size) / ((double)map->capacity);
	if (load_factor >= MAP_LOAD_FACTOR) {
		rehash(map);
	}	
	
	insert(map, map->hash(key), item);
}

bool lx_map_remove(lx_map_t *map, lx_any_t key)
{
	size_t hash = map->hash(key);
	size_t index = 0;

	if (!find_bucket(map, hash, &index))
		return false;
	
	map->entry_states[index] = 0;
	map->size--;

	return true;
}

lx_any_t lx_map_at(const lx_map_t *map, lx_any_t key, lx_any_t default_value)
{
	LX_ASSERT(map, "Invalid map");
	
	size_t hash = map->hash(key);
	size_t index = 0;
	
	if (find_bucket(map, hash, &index)) {
		return ((char *)map->items) + map->element_size * index;
	}
	
	return default_value;
}

bool lx_map_try_get_value(const lx_map_t *map, lx_any_t key, lx_any_t *item)
{
	size_t hash = map->hash(key);
	size_t index = 0;

	if (find_bucket(map, hash, &index)) {
		*item = ((char *)map->items) + map->element_size * index;
		return true;
	}

	return false;
}
