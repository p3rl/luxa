#include <test/luxa/collections/map_tests.h>
#include <luxa/test.h>
#include <luxa/collections/map.h>
#include <luxa/hash.h>

void create_returns_new_hash_map()
{
	// Arrange
	lx_allocator_t *allocator = lx_allocator_default();

	// Act
	lx_map_t *map = lx_map_create(allocator, sizeof(int), lx_string_hash64);

	// Assert
	LX_ASSERT(map, "Invalid map");
	lx_map_destroy(map);
}

void insert_and_at_returns_correct_values()
{
	// Arrange
	lx_map_t *map = lx_map_create(lx_allocator_default(), sizeof(int), lx_string_hash64);

	// Act & Assert
	int a = 41;
	lx_map_insert(map, "a", &a);
	
	int b = 42;
	lx_map_insert(map, "b", &b);

	int *a_ptr = lx_map_at(map, "a", NULL);
	LX_EQUALS(*a_ptr, 41);

	int *b_ptr = lx_map_at(map, "b", NULL);
	LX_EQUALS(*b_ptr, 42);

	lx_map_destroy(map);
}

void get_value_succeeds()
{
	// Arrange
	lx_map_t *map = lx_map_create(lx_allocator_default(), sizeof(int), lx_string_hash64);
	int s = 41;
	lx_map_insert(map, "C99", &s);

	// Act
	int *value;
	bool exists = lx_map_try_get_value(map, "C99", &value);

	// Assert
	LX_EQUALS(*value, 41);
	LX_TRUE(exists);
}

void setup_map_test_fixture()
{
	LX_TEST_FIXTURE_BEGIN("Map");
		LX_ADD_TEST(create_returns_new_hash_map);
		LX_ADD_TEST(insert_and_at_returns_correct_values);
		LX_ADD_TEST(get_value_succeeds);
	LX_TEST_FIXTURE_END();
}
