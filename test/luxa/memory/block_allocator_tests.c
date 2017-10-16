#include <test/luxa/memory/block_allocator_tests.h>
#include <luxa/memory/block_allocator.h>
#include <luxa/test.h>

void create_destroy_block_allocator_succeeds()
{
	// Arrange
	lx_allocator_t *allocator = lx_allocator_default();
	
	// Act & Assert
	lx_allocator_t *block_allocator = lx_block_allocator_create(allocator, sizeof(int), 16);
	lx_block_allocator_destroy(block_allocator);
}

void setup_block_allocator_test_fixture()
{
	LX_TEST_FIXTURE_BEGIN("BlockAllocator")
		LX_ADD_TEST(create_destroy_block_allocator_succeeds);
	LX_TEST_FIXTURE_END()
}