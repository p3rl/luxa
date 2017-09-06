#include <test/luxa/collections/string_tests.h>
#include <luxa/test.h>
#include <luxa/collections/string.h>

void create_string_with_no_allocator_succeeds()
{
	// Arrange & Act
	lx_string_t *s = lx_string_create(NULL);

	// Assert
	LX_ASSERT(s->allocator != NULL, "Invalid allocator");
	LX_EQUALS(s->size, (size_t)0);
	LX_EQUALS(s->capacity, (size_t)16);
}

void create_string_from_string_succeeds()
{
	// Arrange & Act
	lx_string_t *s = lx_string_from_c_str(NULL, "MosDef");

	// Assert
	LX_ASSERT(s->allocator != NULL, "Invalid allocator");
	LX_EQUALS(s->size, (size_t)6);
	LX_EQUALS(s->capacity, (size_t)16);
	LX_TRUE(lx_string_equals_cstr(s, "MosDef"));
}

void string_equals_c_str()
{
	// Arrange
	lx_string_t *s = lx_string_from_c_str(NULL, "MosDef");

	// Act
	bool equals = lx_string_equals_cstr(s, "MosDef");
	bool not_equals = !lx_string_equals_cstr(s, NULL);

	// Assert
	LX_TRUE(equals);
	LX_TRUE(not_equals);
}

void last_of_returns_correct_index()
{
	// Arrange
	const char *s = "Dot Net Perls";
	
	// Act
	size_t i1 = lx_string_last_index_of(s, "N");
	size_t i2 = lx_string_last_index_of(s, "Perls");
	size_t i3 = lx_string_last_index_of(s, "e");
	size_t i4 = lx_string_last_index_of(s, "z");
	size_t i5 = lx_string_last_index_of(s, "D");
	size_t i6 = lx_string_last_index_of(s, "Do");
	
	// Assert
	LX_EQUALS(i1, (size_t)4);
	LX_EQUALS(i2, (size_t)8);
	LX_EQUALS(i3, (size_t)9);
	LX_EQUALS(i4, LX_STRING_NPOS);
	LX_EQUALS(i5, 0);
	LX_EQUALS(i6, 0);
}

void assign_c_str_sets_string()
{
	// Arrange
	const char *c_str = "MosDef";
	lx_string_t *s = lx_string_create(NULL);

	// Act
	lx_string_assign_c_str(s, c_str);

	// Assert
	LX_TRUE(lx_string_equals_cstr(s, "MosDef"));

	// Act
	lx_string_assign_c_str(s, NULL);
	LX_TRUE(lx_string_is_empty(s));
}

void setup_string_test_fixture()
{
	LX_TEST_FIXTURE_BEGIN("String");
		LX_ADD_TEST(create_string_with_no_allocator_succeeds);
		LX_ADD_TEST(create_string_from_string_succeeds);
		LX_ADD_TEST(string_equals_c_str);
		LX_ADD_TEST(last_of_returns_correct_index);
		LX_ADD_TEST(assign_c_str_sets_string);
	LX_TEST_FIXTURE_END();
}