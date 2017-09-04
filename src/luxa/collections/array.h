#ifndef ARRAY_H
#define ARRAY_H

#include <luxa/memory/allocator.h>
#include <string.h>

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

static void lx_array_destroy(lx_array_t *array)
{
	LX_ASSERT(array, "Invaid array");
	lx_free(array->allocator, array->buffer);
	lx_free(array->allocator, array);
}

static void lx_array_push_back(lx_array_t *array, void* element)
{
	LX_ASSERT(array, "Invalid array");
	if (array->size >= array->capacity) {
		array->capacity = array->capacity ? array->capacity * 2 : DEFAULT_ARRAY_CAPACITY;
		lx_realloc(array->allocator, array->buffer, array->element_size * array->capacity);
	}
	memcpy(array->buffer + (array->size * array->element_size), element, array->element_size);
	array->size++;
}

static bool lx_array_is_empty(lx_array_t *array)
{
	LX_ASSERT(array, "Invalid array");
	return array->size == 0;
}

static void *lx_array_begin(lx_array_t *array)
{
	LX_ASSERT(array, "Invalid array");
	return array->size ? (void*)(array->buffer) : NULL;
}

static void *lx_array_end(lx_array_t *array)
{
	LX_ASSERT(array, "Invalid array");
	LX_ASSERT(array->size, "Array is empty");
	return (void*)(array->buffer + (array->size * array->element_size));
}

static void *lx_array_at(lx_array_t *array, size_t index)
{
	LX_ASSERT(array, "Invalid array");
	LX_ASSERT(index < array->size, "Index out of bounds");
	return (void*)(array->buffer + (array->element_size * index));
}

static void lx_array_push_back_int(lx_array_t *array, int value)
{
	LX_ASSERT(array, "Invalid array");
	lx_array_push_back(array, &value);
}

static size_t lx_array_size(lx_array_t *array)
{
	LX_ASSERT(array, "Invalid array");
	return array->size;
}

#define lx_array_for_each(type, ptr, arr)\
	for (type *ptr = lx_array_begin(arr); ptr != lx_array_end(arr); ++ptr)

#endif // ARRAY_H
