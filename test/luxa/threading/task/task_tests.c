#include <test/luxa/threading/task/task_tests.h>
#include <luxa/threading/task/task.h>
#include <luxa/test.h>

typedef struct task_args {
	int a;
	int b;
	int result;
} task_args_t;

void add_numbers(lx_task_t t, task_args_t *args)
{
	args->result = args->a + args->b;
}

void sub_numbers(lx_task_t t, task_args_t *args)
{
	args->result = args->a - args->b;
}

void create_and_start_task_succeeds()
{
	// Arrange
	lx_allocator_t *allocator = lx_allocator_default();
	lx_task_factory_t *task_factory = lx_task_factory_default(allocator);
	task_args_t args = { 41, 42, 0 };
	
	// Act
	lx_task_t t = lx_task_create(task_factory, add_numbers, &args);
	lx_task_start(task_factory, t);
	lx_task_wait(task_factory, t);

	// Assert
	LX_EQUALS(args.result, 83);
}

void contiune_with_task_succeeds()
{
	// Arrange
	lx_allocator_t *allocator = lx_allocator_default();
	lx_task_factory_t *task_factory = lx_task_factory_default(allocator);
	task_args_t args = { 2, 2, 0 };

	// Act
	lx_task_t first_task = lx_task_run(task_factory, add_numbers, &args);
	lx_task_t second_task = lx_task_continue_with(task_factory, first_task, sub_numbers, &args);
	lx_task_wait(task_factory, second_task);

	// Assert
	LX_EQUALS(args.result, 0);
}

void setup_task_test_fixture()
{
	LX_TEST_FIXTURE_BEGIN("Task")
		LX_ADD_TEST(create_and_start_task_succeeds);
		LX_ADD_TEST(contiune_with_task_succeeds);
	LX_TEST_FIXTURE_END();
}