#pragma once

#include <luxa/platform.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_mutex {
	CRITICAL_SECTION critical_section;
} lx_mutex_t;

void lx_mutex_create(lx_mutex_t *mutex);

void lx_mutex_destroy(lx_mutex_t *mutex);

void lx_mutex_lock(lx_mutex_t *mutex);

bool lx_mutex_try_lock(lx_mutex_t *mutex);

void lx_mutex_unlock(lx_mutex_t *mutex);

#ifdef __cplusplus
}
#endif
