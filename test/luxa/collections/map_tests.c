#include <test/luxa/collections/map_tests.h>
#include <luxa/test.h>

void create_returns_new_hash_map()
{

}

void setup_map_test_fixture()
{
	LX_TEST_FIXTURE_BEGIN("Map");
		LX_ADD_TEST(create_returns_new_hash_map);
	LX_TEST_FIXTURE_END();
}
