#include <test/luxa/collections/queue_tests.h>
#include <luxa/collections/queue.h>
#include <luxa/test.h>

void create_returns_new_queue()
{
    // Arrange
    lx_allocator_t *allocator = lx_allocator_default();
 
    // Act
    lx_queue_t *queue = lx_queue_create(allocator, sizeof(int));

    // Assert
    LX_NOT_NULL(queue);
    LX_NOT_NULL(queue->data);
    LX_EQUALS(queue->size, 0);
    LX_EQUALS(queue->capacity, LX_DEFAULT_QUEUE_CAPACITY);
    LX_EQUALS(queue->head, 0);
    LX_EQUALS(queue->tail, LX_DEFAULT_QUEUE_CAPACITY - 1);
}

void enqueue_dequeue_element_succeeds()
{
    // Arrange
    lx_allocator_t *allocator = lx_allocator_default();
    lx_queue_t *queue = lx_queue_create(allocator, sizeof(int));
    size_t num_values = 32;

    // Act
    for (size_t i = 0; i < num_values; ++i) {
        lx_queue_enqueue(queue, &i);
    }
    
    // Assert
    LX_EQUALS(lx_queue_size(queue), num_values);
    LX_TRUE(!lx_queue_is_empty(queue));

    for (size_t i = 0; i < num_values; ++i) {
        int *value = lx_queue_front(queue);
        LX_EQUALS(*value, i);
        lx_queue_dequeue(queue);
    }

    LX_TRUE(lx_queue_is_empty(queue));
    LX_NULL(lx_queue_front(queue));
}

void setup_queue_test_fixture()
{
    LX_TEST_FIXTURE_BEGIN("Queue");
        LX_ADD_TEST(create_returns_new_queue);
        LX_ADD_TEST(enqueue_dequeue_element_succeeds);
    LX_TEST_FIXTURE_END();
}
