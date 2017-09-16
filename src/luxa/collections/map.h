#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <luxa/memory/allocator.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_map lx_map_t;

typedef size_t (*lx_hash_func_t)(const lx_any_t key);

lx_map_t *lx_map_create(lx_allocator_t *allocator, size_t element_size, lx_hash_func_t);

void lx_map_destroy(lx_map_t *map);

void lx_map_insert(lx_map_t *map, lx_any_t key, lx_any_t item);

lx_any_t lx_map_at(const lx_map_t *map, lx_any_t key, lx_any_t default_value);

bool lx_map_try_get_value(const lx_map_t *map, lx_any_t key, lx_any_t *item);

#ifdef __cplusplus
}
#endif

#endif // HASH_MAP_H