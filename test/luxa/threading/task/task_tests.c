#include <test/luxa/threading/task/task_tests.h>
#include <luxa/threading/task/task.h>
#include <luxa/test.h>

typedef struct task_args {
	int value;
	int result;
} task_args_t;

void do_work(lx_task_t t, task_args_t *args)
{
	args->result = args->value;
}

void create_task()
{
	lx_allocator_t *allocator = lx_allocator_default();
	lx_task_factory_t *task_factory = lx_task_factory_default(allocator);

	task_args_t args = { 42, 0 };
	lx_task_t t = lx_task_create(task_factory, do_work, &args);
	lx_task_start(task_factory, t);
	lx_task_wait(task_factory, t);

	lx_task_factory_destroy(task_factory);
}

void setup_task_test_fixture()
{
	LX_TEST_FIXTURE_BEGIN("Task")
		LX_ADD_TEST(create_task);
	LX_TEST_FIXTURE_END();
}