#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <luxa/memory/allocator.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_buffer
{
	lx_allocator_t *allocator;
	void *data;
	size_t size;
	size_t capacity;
} lx_buffer_t;

static inline lx_buffer_t *lx_buffer_create(lx_allocator_t *allocator, size_t capacity)
{
	lx_allocator_t *a = allocator ? allocator : lx_allocator_default();
	lx_buffer_t *buffer = (lx_buffer_t *)lx_alloc(a, sizeof(lx_buffer_t));
	*buffer = (lx_buffer_t) { .allocator = a, .data = NULL, .size = 0, .capacity = capacity };
	
	if (capacity) {
		buffer->data = lx_alloc(a, capacity);
	}

	return buffer;
}

static inline lx_buffer_t *lx_buffer_create_empty(lx_allocator_t *allocator)
{
	return lx_buffer_create(allocator, 0);
}

static inline char *lx_buffer_data(lx_buffer_t *buffer)
{
	LX_ASSERT(buffer, "Invalid buffer");
	return buffer->data;
}

static inline size_t lx_buffer_size(lx_buffer_t *buffer)
{
	LX_ASSERT(buffer, "Invalid buffer");
	return buffer->size;
}

static inline size_t lx_buffer_capacity(lx_buffer_t *buffer)
{
	LX_ASSERT(buffer, "Invalid buffer");
	return buffer->capacity;
}

static inline void lx_buffer_copy_data(lx_buffer_t *dst, const char *src, size_t size)
{
	LX_ASSERT(dst, "Invalid destination buffer");
	
	if (size >= dst->capacity) {
		lx_free(dst->allocator, dst->data);
		dst->capacity = size;
		dst->data = lx_alloc(dst->allocator, dst->capacity);
	}

	dst->size = size;
	memcpy(dst->data, src, size);
}

static inline void lx_buffer_copy(lx_buffer_t *dst, const lx_buffer_t *src)
{
	LX_ASSERT(dst, "Invalid destination buffer");
	LX_ASSERT(src, "Invalid source buffer");
	
	lx_buffer_copy_data(dst, src->data, src->size);
}

static inline void lx_buffer_reserve(lx_buffer_t *buffer, size_t capacity)
{
	if (capacity <= buffer->capacity)
		return;

	lx_buffer_t old = *buffer;
	buffer->data = lx_alloc(buffer->allocator, capacity);
	buffer->capacity = capacity;
	memcpy(buffer->data, old.data, old.size);
}

static inline void lx_buffer_resize(lx_buffer_t *buffer, size_t size)
{
	LX_ASSERT(buffer, "Invalid buffer");

	if (buffer->size == size)
		return;
	
	lx_buffer_reserve(buffer, size);
	if (size > buffer->size)
		memset(((char *)buffer->data) + buffer->size, 0, size - buffer->size);
	
	buffer->size = size;
}

static inline void lx_buffer_clear(lx_buffer_t *buffer)
{
	LX_ASSERT(buffer, "Invalid buffer");
	
	buffer->size = 0;
	memset(buffer->data, 0, buffer->capacity);
}

#ifdef __cplusplus
}
#endif

#endif // _BUFFER_H_