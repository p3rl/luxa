#pragma once

#include <luxa/platform.h>
#include <luxa/memory/allocator.h>

#ifdef __cplusplus
extern "C" {
#endif

static const unsigned DEFAULT_ARRAY_CAPACITY = 16;

typedef struct lx_array
{
	lx_allocator_t *allocator;
	char *buffer;
	size_t element_size;
	size_t size;
	size_t capacity;
} lx_array_t;

static lx_array_t *lx_array_create(lx_allocator_t *allocator, size_t element_size)
{
	LX_ASSERT(allocator, "Invalid allocator");
	LX_ASSERT(element_size > 0, "Element size must be greater than zero");

	lx_array_t *a = (lx_array_t*)lx_alloc(allocator, sizeof(lx_array_t));
	a->allocator = allocator;
	a->buffer = (char*)lx_alloc(allocator, element_size * DEFAULT_ARRAY_CAPACITY);
	a->element_size = element_size;
	a->capacity = DEFAULT_ARRAY_CAPACITY;
	a->size = 0;
	return a;
}

static inline void lx_array_reserve(lx_array_t *array, size_t size)
{
    if (size < array->capacity)
        return;
    
    array->capacity = array->capacity ? lx_max(array->capacity * 2, size) : DEFAULT_ARRAY_CAPACITY;
    array->buffer = lx_realloc(array->allocator, array->buffer, array->element_size * array->capacity);
}

static lx_array_t *lx_array_create_with_size(lx_allocator_t *allocator, size_t element_size, size_t size)
{
	LX_ASSERT(allocator, "Invalid allocator");
	LX_ASSERT(element_size > 0, "Element size must be greater than zero");
	LX_ASSERT(size > 0, "Size must be greater than zero");

	lx_array_t *a = (lx_array_t*)lx_alloc(allocator, sizeof(lx_array_t));
	a->allocator = allocator;
	a->buffer = (char*)lx_alloc(allocator, element_size * size);
	a->element_size = element_size;
	a->capacity = size;
	a->size = size;
	return a;
}

static LX_INLINE void lx_array_destroy(lx_array_t *array)
{
	LX_ASSERT(array, "Invaid array");
	lx_free(array->allocator, array->buffer);
	lx_free(array->allocator, array);
}

static LX_INLINE void lx_array_push_back(lx_array_t *array, lx_any_t element)
{
	LX_ASSERT(array, "Invalid array");

    lx_array_reserve(array, array->size);
	memcpy(array->buffer + (array->size * array->element_size), element, array->element_size);
	array->size++;
}

static LX_INLINE bool lx_array_is_empty(const lx_array_t *array)
{
	LX_ASSERT(array, "Invalid array");
	return array->size == 0;
}

static LX_INLINE lx_any_t lx_array_begin(lx_array_t *array)
{
	LX_ASSERT(array, "Invalid array");
	return (void*)(array->buffer);
}

static LX_INLINE lx_any_t lx_array_end(lx_array_t *array)
{
	LX_ASSERT(array, "Invalid array");
	return (lx_any_t)(array->buffer + (array->size * array->element_size));
}

static LX_INLINE lx_any_t lx_array_at(const lx_array_t *array, size_t index)
{
	LX_ASSERT(array, "Invalid array");
	LX_ASSERT(index < array->size, "Index out of bounds");
	return (lx_any_t)(array->buffer + (array->element_size * index));
}

static LX_INLINE lx_any_t lx_array_pop_back(lx_array_t *array)
{
	LX_ASSERT(array->size, "Array is empty");
	lx_any_t element = lx_array_at(array, array->size - 1);
	array->size--;
	return element;
}

static LX_INLINE void lx_array_push_back_int(lx_array_t *array, int value)
{
	LX_ASSERT(array, "Invalid array");
	lx_array_push_back(array, &value);
}

static LX_INLINE size_t lx_array_size(lx_array_t *array)
{
	LX_ASSERT(array, "Invalid array");
	return array->size;
}

static LX_INLINE size_t lx_array_bytes(lx_array_t *array)
{
	LX_ASSERT(array, "Invalid array");
	return array->size * array->element_size;
}

static LX_INLINE bool lx_array_empty(lx_array_t *array)
{
	LX_ASSERT(array, "Invalid array");
	return array->size == 0;
}

static LX_INLINE bool lx_array_exists(const lx_array_t *array, lx_binary_predicate_t predicate, lx_any_t arg)
{
	LX_ASSERT(array, "Invalid array");

	bool exists = false;
	for (int i = 0; (i < array->size) && !exists; ++i) {
		exists = predicate(lx_array_at(array, i), arg);
	}
	return exists;
}

static LX_INLINE lx_range_t lx_array_range(const lx_array_t *array)
{
	return (lx_range_t) {
		.begin = array->buffer,
		.end = array->buffer + array->size * array->element_size,
		.step_size = array->element_size
	};
}

static LX_INLINE lx_any_t lx_array_find_if(const lx_array_t *array, lx_binary_predicate_t predicate, lx_any_t arg)
{
	LX_ASSERT(array, "Invalid array");

	lx_range_t range = lx_array_range(array);
	char *element = range.begin;
	while (element != range.end) {
		if (predicate(element, arg))
			return element;

		element += range.step_size;
	}

	return NULL;
}

static LX_INLINE bool lx_array_remove_at(lx_array_t *array, size_t index)
{
    if (index >= array->size || array->size == 0)
        return false;

    if (index == 0) {
        memmove(array->buffer, array->buffer + array->element_size, array->element_size * (array->size - 1));
    }
    else if (index != array->size - 1){
        const size_t num_elements = array->size - 1 - index;
        char *dst = array->buffer + array->element_size * index;
        char *src = dst + array->element_size;
        memmove(dst, src, num_elements * array->element_size);
    }

    array->size--;
    return true;
}

static LX_INLINE bool lx_array_remove_if(lx_array_t *array, lx_binary_predicate_t predicate, lx_any_t arg)
{
    LX_ASSERT(array, "Invalid array");
    LX_ASSERT(predicate, "Invalid array");

    size_t i = 0;
    lx_range_t range = lx_array_range(array);
    char *element = range.begin;
    while (element != range.end) {
        if (predicate(element, arg))
            return lx_array_remove_at(array, i);

        element += range.step_size;
        ++i;
    }

    return false;
}

static LX_INLINE void lx_array_copy(lx_array_t *array, lx_any_t data, size_t size)
{
    LX_ASSERT(array, "Invalid array");
    
    lx_array_reserve(array, size);
    memcpy(array->buffer, data, size * array->element_size);
    array->size = size;
}

#define lx_array_for(type, ptr, arr)\
    for (type *ptr = lx_array_begin(arr); ptr != lx_array_end(arr); ++ptr)

#ifdef __cplusplus
}
#endif
