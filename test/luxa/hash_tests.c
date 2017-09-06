#include <test/luxa/hash_tests.h>
#include <luxa/test.h>
#include <luxa/hash.h>
#include <string.h>

void murmur_hash_32_succeeds()
{
	// Arrange
	const char *s = "Some string";

	// Act
	uint32_t hash = lx_murmur_hash_32(s, strlen(s), 0);

	// Assert
	LX_ASSERT(hash != 0, "Invaild hash");
}

void murmur_hash_64_succeeds()
{
	// Arrange
	const char *s = "Some string";

	// Act
	uint64_t hash = lx_murmur_hash_64(s, strlen(s), 0);

	// Assert
	LX_ASSERT(hash != 0, "Invaild hash");
}

void setup_hash_test_fixture()
{
	LX_TEST_FIXTURE_BEGIN("Hash")
		LX_ADD_TEST(murmur_hash_32_succeeds);
		LX_ADD_TEST(murmur_hash_64_succeeds);
	LX_TEST_FIXTURE_END()
}