#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#ifndef NULL
#define NULL ((void*)0);
#endif

typedef enum lx_result
{
	LX_SUCCESS = 0,
	LX_ERROR = 1
} lx_result_t;

#define LX_FAILED(result) result != LX_SUCCESS

#define LX_ASSERT(expr, msg) assert(expr)

#define lx_max(a, b) a > b ? a : b

#define lx_min(a, b) a < b ? a : b

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_H
