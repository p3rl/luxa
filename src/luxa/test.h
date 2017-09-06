#ifndef LX_UNIT_H
#define LX_UNIT_H

#include <stdio.h>

static unsigned num_tests_succeeded = 0;
static unsigned num_tests_failed = 0;
static unsigned test_asserts = 0;

#ifdef __cplusplus
extern "C" {
#endif

#define LX_TEST_FIXTURE_BEGIN(name)\
printf(name);\
printf("\n--------------------------------------------------------------------------------\n");

#define LX_TEST_FIXTURE_END()\
printf("--------------------------------------------------------------------------------\n\n");

#define LX_TEST_ASSERT(ok, a, b, format)\
do {\
	if (!ok) {\
		test_asserts++;\
		printf("%s:%d ("format " != " format")\n", __FILE__, __LINE__, (a), (b));\
	}\
} while(0)

#define LX_EQUALS(a, b) LX_TEST_ASSERT((bool)(a == b), (int)a, (int)b, "%d")

#define LX_NOT_NULL(ptr)\
do {\
	if (ptr == NULL) {\
		test_asserts++;\
		printf("%s:%d (%s is NULL)\n", __FILE__, __LINE__, #ptr);\
	}\
} while(0)

#define LX_TRUE(ok)\
do {\
	if (!ok) {\
		test_asserts++;\
		printf("%s:%d Expression is NOT true\n", __FILE__, __LINE__);\
	}\
} while(0)

#define LX_ADD_TEST(test)\
do {\
	test_asserts = 0;\
	test();\
	printf("%s [%s]\n", #test, test_asserts ? "FAIL" : "OK");\
} while(0)

#ifdef __cplusplus
}
#endif

#endif //LX_UNIT_H
