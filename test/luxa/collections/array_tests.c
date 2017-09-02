#include <test/luxa/collections/array_tests.h>
#include <luxa/test.h>
#include <luxa/collections/array.h>

void create_returns_new_array()
{
	// Arrange
	lx_allocator_t *allocator = lx_default_allocator();

	// Act
	lx_array_t *numbers = lx_array_create(allocator, sizeof(int));

	// Assert
	LX_NOT_NULL(numbers);
	LX_NOT_NULL(numbers->buffer);
	LX_EQUALS(numbers->element_size, sizeof(int));
	LX_EQUALS(numbers->size, 0u);
}

void push_back_adds_value()
{
	// Arrange
	lx_allocator_t *allocator = lx_default_allocator();
	lx_array_t *numbers = lx_array_create(allocator, sizeof(int));

	// Act
	int s = 42;
	lx_array_push_back(numbers, &s);
	int *ps = lx_array_at(numbers, 0);

	// Assert
	LX_EQUALS(numbers->size, 1);
	LX_EQUALS(*ps, 42);
}

void setup_array_test_fixture()
{
	LX_TEST_FIXTURE_BEGIN("Array");
		LX_ADD_TEST(create_returns_new_array);
		LX_ADD_TEST(push_back_adds_value);
	LX_TEST_FIXTURE_END();
}