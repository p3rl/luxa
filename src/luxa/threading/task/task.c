#include <luxa/threading/task/task.h>
#include <luxa/threading/threading.h>
#include <luxa/collections/array.h>

static const uint64_t TASK_ID_MASK = 0x00000000FFFFFFFF;
static const uint64_t TASK_GENERATION_MASK = 0xFFFFFFFF00000000;

typedef struct buffer {
	void *data;
	size_t size;
	size_t capacity;
} buffer_t;

typedef struct task_context {
	lx_task_factory_state_t *factory_state;
	lx_task_t task;
	lx_task_t parent_task;
	lx_task_status_t status;
	lx_task_function_t f;
	lx_any_t arg;
	TP_WORK *tp_work;
} task_context_t;

typedef struct factory_state {
	lx_allocator_t *allocator;
	lx_mutex_t mutex;
	lx_array_t *tasks; // task_context_t
	lx_array_t *free_tasks; // lx_task_t

	TP_CALLBACK_ENVIRON thread_pool_callback_env;
	TP_POOL *thread_pool;
} factory_state_t;

uint32_t task_generation(lx_task_t task)
{
	return (uint32_t)((task & TASK_GENERATION_MASK) >> 32);
}

uint32_t task_index(lx_task_t task)
{
	return (uint32_t)(task & TASK_ID_MASK);
}

lx_task_t task_incremeant_generation(lx_task_t task)
{
	uint64_t generation = task_generation(task) + 1;
	return task | (generation << 32);
}

void execute_task(PTP_CALLBACK_INSTANCE instance, lx_any_t context, PTP_WORK tp_work)
{
	task_context_t *task_context = (task_context_t *)context;
	factory_state_t *s = (factory_state_t *)task_context->factory_state;

	if (task_context->parent_task) {
		task_context_t *parent_task_context = lx_array_at(s->tasks, task_index(task_context->parent_task));
		if (parent_task_context->tp_work) {
			WaitForThreadpoolWorkCallbacks(parent_task_context->tp_work, false);
		}
	}
	
	task_context->status = LX_TASK_STATUS_RUNNING;
	task_context->f(task_context->task, task_context->arg);
	task_context->status = LX_TASK_STATUS_DONE;
	task_context->tp_work = NULL;

	lx_mutex_lock(&s->mutex);
	lx_array_push_back(s->free_tasks, &task_context->task);
	lx_mutex_unlock(&s->mutex);
}

lx_task_factory_state_t *create_factory_state(lx_allocator_t *allocator)
{
	factory_state_t *state = lx_alloc(allocator, sizeof(factory_state_t));
	*state = (factory_state_t) { 0 };
	
	state->allocator = allocator;
	lx_mutex_create(&state->mutex);
	state->tasks = lx_array_create(allocator, sizeof(task_context_t));
	state->free_tasks = lx_array_create(allocator, sizeof(lx_task_t));
	InitializeThreadpoolEnvironment(&state->thread_pool_callback_env);
	state->thread_pool = CreateThreadpool(NULL);

	// Reserve index zero for nil task.
	lx_array_push_back(state->tasks, &(task_context_t) {0});
	
	return (lx_task_factory_state_t *)state;
}

static lx_task_t create_task(lx_task_factory_state_t *state, lx_task_function_t f, lx_any_t arg)
{
	factory_state_t *s = (factory_state_t *)state;
	lx_task_t task = 0;

	lx_mutex_lock(&s->mutex);

	if (lx_array_empty(s->free_tasks)) {
		task = lx_array_size(s->tasks);
		lx_array_push_back(s->tasks, &(task_context_t) { 0 });
	}
	else {
		task = task_incremeant_generation(*((lx_task_t *)lx_array_pop_back(s->free_tasks)));
	}

	lx_mutex_unlock(&s->mutex);

	size_t index = task_index(task);
	task_context_t *task_context = lx_array_at(s->tasks, index);
	task_context->factory_state = state;
	task_context->task = task;
	task_context->parent_task = 0;
	task_context->f = f;
	task_context->arg = arg;
	task_context->status = LX_TASK_STATUS_CREATED;
	task_context->tp_work = CreateThreadpoolWork(execute_task, task_context, &s->thread_pool_callback_env);
	
	return task;
}

static void start_task(lx_task_factory_state_t *state, lx_task_t task)
{
	factory_state_t *s = (factory_state_t *)state;
	task_context_t *task_context = lx_array_at(s->tasks, task_index(task));
	SubmitThreadpoolWork(task_context->tp_work);
}

static lx_task_t continue_with(lx_task_factory_state_t *state, lx_task_t parent_task, lx_task_function_t f, lx_any_t arg)
{
	factory_state_t *s = (factory_state_t *)state;
	lx_task_t task = create_task(state, f, arg);
	task_context_t *ctx = lx_array_at(s->tasks, task_index(task));
	ctx->parent_task = parent_task;
	SubmitThreadpoolWork(ctx->tp_work);

	return task;
}

static void wait(lx_task_factory_state_t *state, lx_task_t task)
{
	factory_state_t *s = (factory_state_t *)state;
	size_t index = task_index(task);
	task_context_t *task_context = lx_array_at(s->tasks, index);
	
	LX_ASSERT(task_context->task == task, "Invalid task");
	WaitForThreadpoolWorkCallbacks(task_context->tp_work, false);
}

lx_task_factory_t *lx_task_factory_default(lx_allocator_t *allocator)
{
	static lx_task_factory_t default_factory = {
		.state = NULL,
		.create_task = create_task,
		.start_task = start_task,
		.continue_with = continue_with,
		.wait = wait
	};

	if (!default_factory.state) {
		default_factory.state = create_factory_state(allocator);
	}

	return &default_factory;
}

void lx_task_factory_destroy_default(lx_task_factory_t *factory)
{
	factory_state_t *s = (factory_state_t *)factory->state;
	lx_array_destroy(s->tasks);
	lx_array_destroy(s->free_tasks);
	lx_mutex_destroy(&s->mutex);
	lx_free(s->allocator, s);
}