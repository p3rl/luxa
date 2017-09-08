#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#ifndef NULL
#define NULL ((void*)0);
#endif

typedef void* lx_any_t;

typedef void (*lx_unary_func_t)(lx_any_t any);

typedef void (*lx_binary_func_t)(lx_any_t any1, lx_any_t any2);

typedef bool (*lx_unary_predicate_t)(lx_any_t any);

typedef bool (*lx_binary_predicate_t)(lx_any_t any1, lx_any_t any2);

typedef lx_any_t (*lx_map_func_t)(lx_any_t any);

typedef enum lx_result
{
	LX_SUCCESS = 0,
	LX_ERROR = 1
} lx_result_t;

#define LX_FAILED(result) result != LX_SUCCESS

#define LX_ASSERT(expr, msg) assert(expr)

#define lx_min(a, b) ((a) < (b) ? (a) : (b))

#define lx_max(a, b) ((a) > (b) ? (a) : (b))

typedef struct lx_range
{
	lx_any_t begin;
	lx_any_t end;
	size_t step_size;
} lx_range_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_H
