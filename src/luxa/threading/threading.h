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

typedef struct lx_thread {
    lx_any_t handle;
    uint64_t id;
} lx_thread_t;

typedef unsigned long (*lx_thread_start_t)(lx_any_t arg);

void lx_thread_create(lx_thread_t *thread, lx_thread_start_t thread_start, lx_any_t arg);

void lx_thread_destroy(lx_thread_t *thread);

bool lx_thread_join(lx_thread_t *thread);

typedef unsigned long lx_thread_local_storage_t;

lx_thread_local_storage_t lx_thread_local_create_storage();

void lx_thread_local_set_value(lx_thread_local_storage_t storage, lx_any_t value);

lx_any_t lx_thread_local_get_value(lx_thread_local_storage_t storage);

#ifdef __cplusplus
}
#endif
