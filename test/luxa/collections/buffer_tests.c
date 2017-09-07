#include <test/luxa/collections/buffer_tests.h>
#include <luxa/test.h>
#include <luxa/collections/buffer.h>

static void create_buffer_returns_new_buffer()
{
	// Arrange & Act
	lx_buffer_t* b = lx_buffer_create(NULL, 256);

	// Assert
	LX_NOT_NULL(b);
	LX_NOT_NULL(b->allocator);
	LX_EQUALS(b->size, (size_t)0);
	LX_EQUALS(b->capacity, (size_t)256);
}

void setup_buffer_tests()
{
	LX_TEST_FIXTURE_BEGIN("Buffer");
		LX_ADD_TEST(create_buffer_returns_new_buffer);
	LX_TEST_FIXTURE_END();
}