#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <luxa/memory/allocator.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_map lx_map_t;

lx_map_t *lx_map_create(lx_allocator_t *allocator, size_t element_size);

void lx_map_destroy(lx_map_t *map);

void lx_map_insert(lx_map_t *map, uint64_t key, lx_any_t item);

lx_any_t lx_map_get(lx_map_t *map, uint64_t key);

lx_any_t lx_map_get_or_default(lx_map_t *map, uint64_t key, lx_any_t default_value);

#ifdef __cplusplus
}
#endif

#endif // HASH_MAP_H