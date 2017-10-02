#include <luxa/threading/task/task.h>
#include <luxa/threading/threading.h>
#include <luxa/collections/array.h>

typedef struct buffer {
	void *data;
	size_t size;
	size_t capacity;
} buffer_t;

typedef struct task_state {
	lx_task_status_t status;
	lx_task_function_t f;
	lx_any_t arg;
	lx_any_t result;
	TP_WORK *tp_work;
} task_state_t;

typedef struct factory_state {
	lx_allocator_t *allocator;
	lx_mutex_t mutex;
	lx_array_t *free_tasks;

	TP_CALLBACK_ENVIRON thread_pool_callback_env;
	TP_POOL *thread_pool;
	
	buffer_t buffer;
	lx_task_t *parent;
	lx_task_t *first_child;
	lx_task_t *next_sibling;
	task_state_t *task_state;
} factory_state_t;

typedef struct thread_pool_work {
	factory_state_t *factory_state;
	lx_task_t task;
} thread_pool_work_t;

void execute_task(PTP_CALLBACK_INSTANCE instance, lx_any_t context, PTP_WORK tp_work)
{
	thread_pool_work_t *tpw = (thread_pool_work_t *)context;
	task_state_t *task_state = &tpw->factory_state->task_state[tpw->task];
	task_state->f(tpw->task, task_state->arg);
}

void allocate(factory_state_t *state, size_t capacity)
{
	const size_t field_size = sizeof(lx_task_t) * 3 + sizeof(task_state_t);

	buffer_t old = state->buffer;
	state->buffer.data = lx_alloc(state->allocator, field_size * capacity);
	state->buffer.capacity = capacity;
	memset(state->buffer.data, 0, field_size * capacity);
	
	memcpy(state->buffer.data, old.data, field_size * old.size);

	state->parent = (lx_task_t *)state->buffer.data;
	state->first_child = state->parent + capacity;
	state->next_sibling = state->first_child + capacity;
	state->task_state = (task_state_t *)(state->next_sibling + capacity);

	if (!state->buffer.size) {
		state->buffer.size = 1;
	}
}

lx_task_factory_state_t *create_factory_state(lx_allocator_t *allocator)
{
	factory_state_t *state = lx_alloc(allocator, sizeof(factory_state_t));
	*state = (factory_state_t) { 0 };
	
	state->allocator = allocator;
	lx_mutex_create(&state->mutex);
	state->free_tasks = lx_array_create(allocator, sizeof(lx_task_t));
	InitializeThreadpoolEnvironment(&state->thread_pool_callback_env);
	state->thread_pool = CreateThreadpool(NULL);

	allocate(state, 128);
	
	return (lx_task_factory_state_t *)state;
}

static lx_task_t create_task(lx_task_factory_state_t *state, lx_task_function_t f, lx_any_t arg)
{
	factory_state_t *s = (factory_state_t *)state;

	lx_mutex_lock(&s->mutex);
	
	if (s->buffer.size >= s->buffer.capacity)
		allocate(s, s->buffer.capacity * 2);
	
	lx_task_t task = s->buffer.size;
	s->buffer.size++;
	
	lx_mutex_unlock(&s->mutex);

	s->parent[task] = 0;
	s->first_child[task] = 0;
	s->next_sibling[task] = 0;

	thread_pool_work_t tpw = { s, task };

	s->task_state[task] = (task_state_t) {
		.status = LX_TASK_STATUS_CREATED,
		.f = f,
		.arg = arg,
		.tp_work = CreateThreadpoolWork(execute_task, &tpw, &s->thread_pool_callback_env),
		.result = NULL
	};

	return task;
}

static void start_task(lx_task_factory_state_t *state, lx_task_t task)
{
	factory_state_t *s = (factory_state_t *)state;
	SubmitThreadpoolWork(s->task_state[task].tp_work);
}

static lx_task_t continue_with(lx_task_factory_state_t *state, lx_task_t parent_task, lx_task_function_t f, lx_any_t arg)
{
	return 1;
}

static void wait(lx_task_factory_state_t *state, lx_task_t task)
{
	factory_state_t *s = (factory_state_t *)state;
	WaitForThreadpoolWorkCallbacks(s->task_state[task].tp_work, false);
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
	lx_array_destroy(s->free_tasks);
	lx_mutex_destroy(&s->mutex);
	lx_free(s->allocator, s->buffer.data);
	lx_free(s->allocator, s);
}