#include <test/luxa/collections/array_tests.h>
#include <luxa/test.h>
#include <luxa/collections/array.h>

void create_returns_new_array()
{
	// Arrange
	lx_allocator_t *allocator = lx_allocator_default();

	// Act
	lx_array_t *numbers = lx_array_create(allocator, sizeof(int));

	// Assert
	LX_NOT_NULL(numbers);
	LX_NOT_NULL(numbers->buffer);
	LX_EQUALS(numbers->element_size, sizeof(int));
	LX_EQUALS(numbers->size, 0u);

    lx_array_destroy(numbers);
}

void push_back_adds_value()
{
	// Arrange
	lx_allocator_t *allocator = lx_allocator_default();
	lx_array_t *numbers = lx_array_create(allocator, sizeof(int));

	// Act
	int s = 42;
	lx_array_push_back(numbers, &s);
	int *ps = lx_array_at(numbers, 0);

	// Assert
	LX_EQUALS(numbers->size, 1);
	LX_EQUALS(*ps, 42);

    lx_array_destroy(numbers);
}

void push_back_many_values_succeeds()
{
    // Arrange
    lx_allocator_t *allocator = lx_allocator_default();
    lx_array_t *numbers = lx_array_create(allocator, sizeof(int));

    const size_t num_numbers = 128;

    // Act
    for (int i = 0; i < num_numbers; ++i) {
        lx_array_push_back_int(numbers, i);
    }

    // Assert
    for (size_t i = 0; i < num_numbers; ++i) {
        int *value = lx_array_at(numbers, i);
        LX_EQUALS(i, *value);
    }

    lx_array_destroy(numbers);
}

void remove_at_removes_first_element()
{
    // Arrange
    lx_allocator_t *allocator = lx_allocator_default();
    lx_array_t *numbers = lx_array_create(allocator, sizeof(int));

    // Act
    lx_array_push_back_int(numbers, 1);
    
    // Assert
    lx_array_remove_at(numbers, 0);
    LX_EQUALS(0, lx_array_size(numbers));

    lx_array_destroy(numbers);
}

void remove_at_removes_last_element()
{
    // Arrange
    lx_allocator_t *allocator = lx_allocator_default();
    lx_array_t *numbers = lx_array_create(allocator, sizeof(int));
    lx_array_push_back_int(numbers, 1);
    lx_array_push_back_int(numbers, 2);

    // Act
    lx_array_remove_at(numbers, 1);

    // Assert
    LX_EQUALS(1, lx_array_size(numbers));
    int *first = lx_array_at(numbers, 0);
    LX_EQUALS(1, *first);

    lx_array_destroy(numbers);
}

void remove_at_removes_middle_element()
{
    // Arrange
    lx_allocator_t *allocator = lx_allocator_default();
    lx_array_t *numbers = lx_array_create(allocator, sizeof(int));
    lx_array_push_back_int(numbers, 1);
    lx_array_push_back_int(numbers, 2);
    lx_array_push_back_int(numbers, 3);
    lx_array_push_back_int(numbers, 4);

    // Act
    lx_array_remove_at(numbers, 1); // Remove 2
    lx_array_remove_at(numbers, 1); // Remove 3

    // Assert
    LX_EQUALS(2, lx_array_size(numbers));
    int *first = lx_array_at(numbers, 0);
    int *last = lx_array_at(numbers, 1);
    LX_EQUALS(1, *first);
    LX_EQUALS(4, *last);

    lx_array_destroy(numbers);
}

void setup_array_test_fixture()
{
	LX_TEST_FIXTURE_BEGIN("Array");
		LX_ADD_TEST(create_returns_new_array);
		LX_ADD_TEST(push_back_adds_value);
        LX_ADD_TEST(push_back_many_values_succeeds);
        LX_ADD_TEST(remove_at_removes_first_element);
        LX_ADD_TEST(remove_at_removes_last_element);
        LX_ADD_TEST(remove_at_removes_middle_element);
	LX_TEST_FIXTURE_END();
}