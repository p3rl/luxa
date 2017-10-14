#include <test/luxa/threading/task/task_tests.h>
#include <luxa/threading/task/task.h>
#include <luxa/chrono.h>
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

void empty_task(lx_task_t t, lx_any_t no_args)
{
}

void create_and_start_task_succeeds()
{
	//// Arrange
	//lx_allocator_t *allocator = lx_allocator_default();
	//lx_task_factory_t *task_factory = lx_task_factory_default(allocator);
	//task_args_t args = { 41, 42, 0 };
	//
	//// Act
	//lx_task_t t = lx_task_create(task_factory, add_numbers, &args);
	//lx_task_start(task_factory, t);
	//lx_task_wait(task_factory, t);

	//// Assert
	//LX_EQUALS(args.result, 83);
}

void contiune_with_task_succeeds()
{
	//// Arrange
	//lx_allocator_t *allocator = lx_allocator_default();
	//lx_task_factory_t *task_factory = lx_task_factory_default(allocator);
	//task_args_t args = { 2, 2, 0 };

	//// Act
	//lx_task_t first_task = lx_task_run(task_factory, add_numbers, &args);
	//lx_task_t second_task = lx_task_continue_with(task_factory, first_task, sub_numbers, &args);
	//lx_task_wait(task_factory, second_task);

	//// Assert
	//LX_EQUALS(args.result, 0);
}

void performance_test()
{
	//lx_highres_clock_t clock;
	//lx_highres_clock_create(&clock);

	//lx_allocator_t *allocator = lx_allocator_default();
	//lx_task_factory_t *factory = lx_task_factory_default(allocator);

	//uint64_t start = lx_highres_clock_now();

	//uint32_t num_tasks = 65000;
	//for (uint32_t i = 0; i < num_tasks; ++i) {
	//	lx_task_run(factory, empty_task, NULL);
	//}
	//
	//uint64_t end = lx_highres_clock_now();
	//double milliseconds = lx_highres_clock_milliseconds(&clock, end - start);

	//printf("%+F\n", milliseconds);
}

void setup_task_test_fixture()
{
	LX_TEST_FIXTURE_BEGIN("Task")
		LX_ADD_TEST(create_and_start_task_succeeds);
		LX_ADD_TEST(contiune_with_task_succeeds);
		LX_ADD_TEST(performance_test);
	LX_TEST_FIXTURE_END();
}