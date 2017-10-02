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
