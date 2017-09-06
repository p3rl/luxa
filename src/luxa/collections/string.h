#ifndef STRING_H
#define STRING_H

#include <luxa/memory/allocator.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static const unsigned DEFAULT_STRING_CAPACITY = 16;
static const size_t LX_STRING_NPOS = (size_t)-1;

typedef struct lx_string
{
	lx_allocator_t *allocator;
	char *s;
	size_t size;
	size_t capacity;
} lx_string_t;

static inline lx_string_t *lx_string_create(lx_allocator_t *allocator)
{
	lx_allocator_t *a = allocator ? allocator : lx_default_allocator();
	lx_string_t *string = lx_alloc(a, sizeof(lx_string_t));
	*string = (lx_string_t) { .allocator = a, .size = 0, .capacity = DEFAULT_STRING_CAPACITY };
	string->s = lx_alloc(a, string->capacity);
	string->s[0] = '\0';
	return string;
}

static inline lx_string_t *lx_string_from_c_str(lx_allocator_t *allocator, const char *s)
{
	LX_ASSERT(s, "Invalid string");
	const size_t size = strlen(s);
	lx_allocator_t *a = allocator ? allocator : lx_default_allocator();
	lx_string_t *string = lx_alloc(a, sizeof(lx_string_t));
	*string = (lx_string_t) { .allocator = a, .size = size, .capacity = lx_max(size + 1, DEFAULT_STRING_CAPACITY) };
	string->s = lx_alloc(a, string->capacity);
	memcpy(string->s, s, size);
	string->s[string->size] = '\0';
	return string;
}

static inline bool lx_string_equals_cstr(const lx_string_t *string, const char *c_str)
{
	LX_ASSERT(string, "Invalid string");
	return string->size && c_str != NULL && strcmp(string->s, c_str) == 0;
}

static inline lx_string_t *lx_string_clear(lx_string_t *string)
{
	LX_ASSERT(string, "Invalid string");
	string->s[0] = '\0';
	string->size = 0;
	return string;
}

static inline bool lx_string_is_empty(const lx_string_t *string)
{
	LX_ASSERT(string, "Invalid string");
	return string->size == 0;
}

static inline lx_string_t *lx_string_assign_c_str(lx_string_t *string, const char *c_str)
{
	LX_ASSERT(string, "Invalid string");
	size_t size = c_str != NULL ? strlen(c_str) : 0;
	if (!size)
		return lx_string_clear(string);

	if (size + 1 >= string->capacity) {
		lx_free(string->allocator, string->s);
		string->capacity *= 2;
		string->s = lx_alloc(string->allocator, string->capacity);
	}

	memcpy(string->s, c_str, size);
	string->size = size;
	string->s[size] = '\0';

	return string;
}

static inline size_t lx_string_last_index_of(const char *s, const char *substring)
{
	LX_ASSERT(s, "Invalid string");
	LX_ASSERT(substring, "Invalid substring");

	int size = (int)strlen(s);
	int sub_size = (int)strlen(substring);

	if (sub_size > size || sub_size == 0 || size == 0)
		return LX_STRING_NPOS;
	
	
	for (int i = size - 1; i >= 0; --i) {
		if (s[i] == substring[sub_size - 1]) {
			if (sub_size == 1)
				return i;

			bool match = true;
			int j = i - 1;
			int k = sub_size - 2;
			while (j >= 0 && k >= 0 && match) {
				match = s[j--] == substring[k--];
			}

			if (match)
				return j + 1;
		}
	}

	return LX_STRING_NPOS;
}

#ifdef __cplusplus
}
#endif

#endif //STRING_H
