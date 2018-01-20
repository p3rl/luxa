#include <luxa/threading/threading.h>

void lx_mutex_create(lx_mutex_t *mutex)
{
	LX_ASSERT(mutex, "Invalid mutex");
	
	*mutex = (lx_mutex_t) { 0 };
	InitializeCriticalSection(&mutex->critical_section);
}

void lx_mutex_destroy(lx_mutex_t *mutex)
{
	LX_ASSERT(mutex, "Invalid mutex");

	DeleteCriticalSection(&mutex->critical_section);
}

void lx_mutex_lock(lx_mutex_t *mutex)
{
	LX_ASSERT(mutex, "Invalid mutex");

	EnterCriticalSection(&mutex->critical_section);
}

bool lx_mutex_try_lock(lx_mutex_t *mutex)
{
	LX_ASSERT(mutex, "Invalid mutex");

	return TryEnterCriticalSection(&mutex->critical_section);
}

void lx_mutex_unlock(lx_mutex_t *mutex)
{
	LX_ASSERT(mutex, "Invalid mutex");

	LeaveCriticalSection(&mutex->critical_section);
}

void lx_thread_create(lx_thread_t *thread, lx_thread_start_t thread_start, lx_any_t arg)
{
    LX_ASSERT(thread, "Invalid thread");
    LX_ASSERT(thread_start, "Invalid thread start routine");

    unsigned long id;
    thread->handle = CreateThread(NULL, 0, thread_start, arg, 0, &id);
    thread->id = (uint64_t)id;
}

void lx_thread_destroy(lx_thread_t *thread)
{
    LX_ASSERT(thread, "Invalid thread");
    CloseHandle(thread->handle);
}

bool lx_thread_join(lx_thread_t *thread)
{
	return WaitForSingleObject(thread->handle, INFINITE) == WAIT_OBJECT_0;
}

lx_thread_local_storage_t lx_thread_local_create_storage()
{
    unsigned long storage = TlsAlloc();
    LX_ASSERT(storage != TLS_OUT_OF_INDEXES, "Invalid thread local storage");
    return storage;
}

void lx_thread_local_destroy_storage(lx_thread_local_storage_t storage)
{
    TlsFree(storage);
}

void lx_thread_local_set_value(lx_thread_local_storage_t storage, lx_any_t value)
{
    LX_ASSERT(storage != TLS_OUT_OF_INDEXES, "Invalid thread local storage");
    TlsSetValue(storage, value);
}

lx_any_t lx_thread_local_get_value(lx_thread_local_storage_t storage)
{
    LX_ASSERT(storage != TLS_OUT_OF_INDEXES, "Invalid thread local storage");
    return TlsGetValue(storage);
}

void lx_atomic_increment_32(volatile int32_t *value)
{
	InterlockedIncrement((long *)value);
}

void lx_atomic_decrement_32(volatile int32_t *value)
{
	InterlockedDecrement((long *)value);
}

int32_t lx_atomic_exchange_32(volatile int32_t *dst, int32_t exchange, int32_t comparand)
{
	return (int32_t)InterlockedCompareExchange((long *)dst, exchange, comparand);
}
