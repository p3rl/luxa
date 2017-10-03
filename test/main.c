#include <stdio.h>
#include <luxa/test.h>
#include <test/luxa/collections/array_tests.h>
#include <test/luxa/collections/string_tests.h>
#include <test/luxa/collections/buffer_tests.h>
#include <test/luxa/collections/map_tests.h>
#include <test/luxa/hash_tests.h>
#include <test/luxa/renderer/scene_tests.h>
#include <test/luxa/math/math_tests.h>
#include <test/luxa/threading/task/task_tests.h>

int main(int argc, char **argv)
{
	setup_array_test_fixture();
	setup_hash_test_fixture();
	setup_string_test_fixture();
	setup_buffer_tests();
	setup_map_test_fixture();
    setup_scene_test_fixture();
	setup_math_test_fixture();
	setup_task_test_fixture();
	return 0;
}