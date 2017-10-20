#include <luxa/threading/task/task.h>
#include <luxa/threading/threading.h>
#include <luxa/collections/array.h>
#include <luxa/collections/queue.h>
#include <luxa/memory/block_allocator.h>

struct lx_task {
	lx_task_function_t f;
	lx_any_t arg;
	lx_task_t *parent;
	int32_t unfinishedTasks;
};

typedef struct task_worker {
	lx_allocator_t *task_allocator;
	lx_queue_t *queue;
    lx_mutex_t mutex;
    lx_thread_t thread;
} task_worker_t;

typedef struct factory_state {
    lx_allocator_t *allocator;
    lx_array_t *workers; // task_worker_t
    lx_thread_local_storage_t local_storage;
    volatile bool process_tasks;
} factory_state_t;

typedef struct task_worker_args {
    lx_task_factory_t *task_factory;
    task_worker_t* worker;
} task_worker_args_t;

lx_task_t *try_dequeue_task(task_worker_t *worker)
{
	lx_task_t *task = NULL;

	lx_mutex_lock(&worker->mutex);

	if (!lx_queue_is_empty(worker->queue)) {
		task = *((lx_task_t **)lx_queue_front(worker->queue));
		lx_queue_dequeue(worker->queue);
	}

	lx_mutex_unlock(&worker->mutex);

	return task;
}

lx_task_t *steal_task(task_worker_t *global_worker, task_worker_t *local_worker)
{
	return global_worker != local_worker ? try_dequeue_task(global_worker) : NULL;
}

lx_task_t *next_task(lx_task_factory_t *factory, task_worker_t *local_worker)
{
	factory_state_t *state = (factory_state_t *)factory->state;
	
	// Try dequeue from thread local queue
	lx_task_t *task = try_dequeue_task(local_worker);
	if (task)
		return task;

    // Steel work from another worker
	int i = rand() % lx_array_size(state->workers);
	task_worker_t *global_worker = lx_array_at(state->workers, (size_t)i);

	return steal_task(global_worker, local_worker);
}

void execute_next_task(lx_task_factory_t *factory, task_worker_t *worker)
{
	lx_task_t *task = next_task(factory, worker);
	if (!task)
		return;

	task->f(factory, task, task->arg);
	lx_atomic_decrement_32(&task->unfinishedTasks);
}

unsigned long do_work(task_worker_args_t *args)
{
    lx_task_factory_t *factory = args->task_factory;
    factory_state_t *state = (factory_state_t *)factory->state;
    task_worker_t *worker = args->worker;
    lx_free(state->allocator, args);

    lx_thread_local_set_value(state->local_storage, worker);
    
    while (state->process_tasks) {
		execute_next_task(factory, worker);
	}
    
    return 0;
}

task_worker_t *create_task_worker(lx_task_factory_t *task_factory)
{
    factory_state_t *state = (factory_state_t *)task_factory->state;
    task_worker_t * worker = lx_alloc(state->allocator, sizeof(task_worker_t));
    *worker = (task_worker_t) { 0 };

	worker->task_allocator = lx_block_allocator_create(state->allocator, sizeof(lx_task_t), 1024);
	worker->queue = lx_queue_create(state->allocator, sizeof(lx_task_t*));
    lx_mutex_create(&worker->mutex);

	return worker;
}

void destroy_task_worker(lx_allocator_t *allocator, task_worker_t *worker)
{
	lx_block_allocator_destroy(worker->task_allocator);
	lx_queue_destroy(worker->queue);
    lx_mutex_destroy(&worker->mutex);
    lx_thread_destroy(&worker->thread);
}

factory_state_t *create_factory_state(lx_task_factory_t *task_factory, lx_allocator_t *allocator, size_t num_threads)
{
    factory_state_t *state = lx_alloc(allocator, sizeof(factory_state_t));
    *state = (factory_state_t) {
        .allocator = allocator,
        .workers = lx_array_create(allocator, sizeof(task_worker_t)),
        .local_storage = lx_thread_local_create_storage(),
		.process_tasks = true
    };

	task_factory->state = (lx_task_factory_state_t *)state;

    for (size_t i = 0; i < num_threads; ++i) {
        task_worker_t *worker = create_task_worker(task_factory);
		lx_array_push_back(state->workers, worker);

		task_worker_args_t *args = lx_alloc(state->allocator, sizeof(task_worker_args_t));
		*args = (task_worker_args_t) { task_factory, worker };

		lx_thread_create(&worker->thread, do_work, args);
    }

	return state;
}

static lx_task_t *create_task(lx_task_factory_t *factory, lx_task_function_t f, lx_any_t arg)
{
    LX_ASSERT(factory, "Invalid factory");
    LX_ASSERT(f, "Invalid task function");

	factory_state_t *s = (factory_state_t *)factory->state;
	task_worker_t *worker = lx_thread_local_get_value(s->local_storage);
	
	lx_task_t *task = lx_alloc(worker->task_allocator, sizeof(lx_task_t));
	*task = (lx_task_t) { .f = f, .arg = arg, .parent = NULL, .unfinishedTasks = 1 };

	return task;
}

static void start_task(lx_task_factory_t *factory, lx_task_t *task)
{
    LX_ASSERT(factory, "Invalid factory");
    LX_ASSERT(task, "Invalid task");

	factory_state_t *s = (factory_state_t *)factory->state;
	task_worker_t *worker = lx_thread_local_get_value(s->local_storage);
	
	lx_mutex_lock(&worker->mutex);
	lx_queue_enqueue(worker->queue, &task);
	lx_mutex_unlock(&worker->mutex);
}

static lx_task_t *continue_with(lx_task_factory_t *factory, lx_task_t *parent, lx_task_function_t f, lx_any_t arg)
{
	LX_ASSERT(factory, "Invalid factory");
	LX_ASSERT(parent, "Invalid parent task");
	LX_ASSERT(f, "Invalid task function");

	lx_task_t *child = create_task(factory, f, arg);
	child->parent = parent;

	return child;
}

static void wait(lx_task_factory_t *factory, lx_task_t *task)
{
	factory_state_t *s = (factory_state_t *)factory->state;
	task_worker_t *worker = lx_thread_local_get_value(s->local_storage);
	
	while (lx_atomic_exchange_32(&task->unfinishedTasks, 0, 0) != 0) {
        execute_next_task(factory, worker);
	}
}

lx_task_factory_t *lx_task_factory_default(lx_allocator_t *allocator, size_t num_threads)
{
    static lx_task_factory_t default_factory = {
        .state = NULL,
        .create_task = create_task,
        .start_task = start_task,
		.continue_with = continue_with,
        .wait = wait
    };

    if (!default_factory.state) {
        factory_state_t *state = create_factory_state(&default_factory, allocator, num_threads);

		// Create worker for calling thread
		task_worker_t *worker = create_task_worker(&default_factory);
		lx_array_push_back(state->workers, worker);
		lx_thread_local_set_value(state->local_storage, worker);
    }
    
    return &default_factory;
}

void lx_task_factory_destroy_default(lx_task_factory_t *factory)
{
	LX_ASSERT(factory, "Invalid task factory");

	factory_state_t *state = (factory_state_t *)factory->state;
	factory->state = NULL;

	if (!state)
		return;

	state->process_tasks = false;
	lx_array_for(task_worker_t, *tw, state->workers) {
		task_worker_t *worker = *tw;
		lx_thread_join(&worker->thread);
		destroy_task_worker(state->allocator, worker);
	}

	lx_array_destroy(state->workers);
	//lx_thread_local_destroy_storage(state->local_storage);

	*state = (factory_state_t) { 0 };
}
