#include <test/luxa/threading/threading_tests.h>
#include <luxa/threading/threading.h>
#include <luxa/test.h>

typedef struct thread_arg { int i; } thread_arg_t;

static lx_thread_local_storage_t thred_local_storage;

unsigned long thread_start(thread_arg_t *arg)
{
    arg->i = arg->i * arg->i;
    return 0;
}

void thread_local_add_value()
{
    int *value = lx_thread_local_get_value(thred_local_storage);
    *value = *value + 1;
    lx_thread_local_set_value(thred_local_storage, value);
}

unsigned long thread_local_start(int *value)
{
    lx_thread_local_set_value(thred_local_storage, value);
    thread_local_add_value();
    return 0;
}

void create_join_thread_succeeds()
{
    // Arrange
    thread_arg_t arg = { 10 };
    lx_thread_t thread;
    
    // Act
    lx_thread_create(&thread, thread_start, &arg);
    bool ok = lx_thread_join(&thread);
    lx_thread_destroy(&thread);
    
    // Assert
    LX_EQUALS(100, arg.i);
    LX_ASSERT(ok, "Failed to join thread");
}

void test_thread_local_storage()
{
    // Arrange
    lx_thread_t thread_a, thread_b;
    thred_local_storage = lx_thread_local_create_storage();
    
    int a = 42;
    int b = 62;

    // Act
    lx_thread_create(&thread_a, thread_local_start, &a);
    lx_thread_create(&thread_b, thread_local_start, &b);
    lx_thread_join(&thread_a);
    lx_thread_join(&thread_b);

    // Assert
    LX_EQUALS(a, 43);
    LX_EQUALS(b, 63);
}

void setup_threading_test_fixture()
{
    LX_TEST_FIXTURE_BEGIN("Thread")
        LX_ADD_TEST(create_join_thread_succeeds);
        LX_ADD_TEST(test_thread_local_storage);
    LX_TEST_FIXTURE_END();
}
