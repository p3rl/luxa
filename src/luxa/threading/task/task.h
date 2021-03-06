#pragma once

#include <luxa/platform.h>
#include <luxa/memory/allocator.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_task lx_task_t;

typedef struct lx_task_factory lx_task_factory_t;

typedef void (*lx_task_function_t)(lx_task_factory_t *task_factory, lx_task_t *task, lx_any_t task_argument);

typedef struct lx_task_factory_state lx_task_factory_state_t;

typedef struct lx_task_factory {
	lx_task_factory_state_t *state;
	
	lx_task_t *(*create_task)(lx_task_factory_t *self, lx_task_function_t task_func, lx_any_t task_argument);

	void (*start_task)(lx_task_factory_t *self, lx_task_t *task);

	lx_task_t *(*continue_with)(lx_task_factory_t *self, lx_task_t *parent_task, lx_task_function_t task_func, lx_any_t task_argument);

	void (*wait)(lx_task_factory_t *self, lx_task_t *task);

} lx_task_factory_t;

static LX_INLINE lx_task_t *lx_task_create(lx_task_factory_t *factory, lx_task_function_t task_func, lx_any_t task_argument)
{
	return factory->create_task(factory, task_func, task_argument);
}

static LX_INLINE void lx_task_start(lx_task_factory_t *factory, lx_task_t *task)
{
	factory->start_task(factory, task);
}

static LX_INLINE lx_task_t *lx_task_run(lx_task_factory_t *factory, lx_task_function_t task_func, lx_any_t task_argument)
{
	lx_task_t *task = factory->create_task(factory, task_func, task_argument);
	factory->start_task(factory, task);
	return task;
}

static LX_INLINE lx_task_t *lx_task_continue_with(lx_task_factory_t *factory, lx_task_t *parent_task, lx_task_function_t task_func, lx_any_t task_argument)
{
	return factory->continue_with(factory, parent_task, task_func, task_argument);
}

static LX_INLINE void lx_task_wait(lx_task_factory_t *factory, lx_task_t *task)
{
	factory->wait(factory, task);
}

lx_task_factory_t *lx_task_factory_default(lx_allocator_t *allocator, size_t num_threads);

void lx_task_factory_destroy_default(lx_task_factory_t *factory);

#ifdef __cplusplus
}
#endif
