#include <stdio.h>
#include <luxa/test.h>
#include <test/luxa/collections/array_tests.h>
#include <test/luxa/collections/string_tests.h>
#include <test/luxa/hash_tests.h>

int main(int argc, char **argv)
{
	setup_array_test_fixture();
	setup_hash_test_fixture();
	setup_string_test_fixture();
	return 0;
}