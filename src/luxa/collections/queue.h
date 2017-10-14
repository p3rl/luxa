#pragma once

#include <luxa/platform.h>
#include <luxa/memory/allocator.h>

#ifdef __cplusplus
extern "C" {
#endif

static const unsigned LX_DEFAULT_QUEUE_CAPACITY = 16;

typedef struct lx_queue {
    lx_allocator_t *allocator;
    int8_t *data;
    size_t element_size;
    size_t size;
    size_t capacity;
    size_t head;
    size_t tail;
} lx_queue_t;

static LX_INLINE lx_queue_t *lx_queue_create(lx_allocator_t *allocator, size_t element_size)
{
    LX_ASSERT(allocator, "Invalid allocator");
    LX_ASSERT(element_size, "Invalid element_size");

    lx_queue_t *queue = lx_alloc(allocator, sizeof(lx_queue_t));
    *queue = (lx_queue_t) {
        .allocator = allocator,
        .data = (int8_t *) lx_alloc(allocator, LX_DEFAULT_QUEUE_CAPACITY * element_size),
        .element_size = element_size,
        .size = 0,
        .capacity = LX_DEFAULT_QUEUE_CAPACITY,
        .head = 0,
        .tail = LX_DEFAULT_QUEUE_CAPACITY - 1
    };

    return queue;
}

static LX_INLINE void lx_queue_destroy(lx_queue_t *queue)
{
    LX_ASSERT(queue, "Invalid queue");

    lx_free(queue->allocator, queue->data);
    *queue = (lx_queue_t) { 0 };
}

static LX_INLINE void lx_queue_reserve(lx_queue_t *queue, size_t size)
{
    LX_ASSERT(queue, "Invalid queue");

    if (size < queue->capacity)
        return;

    queue->capacity = lx_max(queue->capacity * 2, size);
    lx_realloc(queue->allocator, queue->data, queue->element_size * queue->capacity);
}

static LX_INLINE void lx_queue_enqueue(lx_queue_t *queue, lx_any_t element)
{
    LX_ASSERT(queue, "Invalid queue");

    lx_queue_reserve(queue, queue->size);
    queue->tail = (queue->tail + 1) % queue->capacity;
    memcpy(queue->data + (queue->tail * queue->element_size), element, queue->element_size);
    ++queue->size;
}

static LX_INLINE lx_any_t lx_queue_front(lx_queue_t *queue)
{
    LX_ASSERT(queue, "Invalid queue");

    return queue->size ? queue->data + (queue->head * queue->element_size) : NULL;
}

static LX_INLINE void lx_queue_dequeue(lx_queue_t *queue)
{
    LX_ASSERT(queue, "Invalid queue");
    LX_ASSERT(queue->size, "Queue is empty");

    queue->head = (queue->head + 1) % queue->capacity;
    --queue->size;
}

static LX_INLINE size_t lx_queue_size(lx_queue_t *queue)
{
    LX_ASSERT(queue, "Invalid queue");

    return queue->size;
}

static LX_INLINE bool lx_queue_is_empty(lx_queue_t *queue)
{
    LX_ASSERT(queue, "Invalid queue");

    return queue->size == 0;
}

#ifdef __cplusplus
}
#endif
