#include <luxa/chrono.h>
#include <windows.h>

void lx_highres_clock_create(lx_highres_clock_t *clock)
{
	LARGE_INTEGER frequency = { 0 };
	QueryPerformanceFrequency(&frequency);
	clock->cpu_tick_delta = 1.0 / (double)frequency.QuadPart;
}

int64_t lx_highres_clock_now()
{
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
	return (int64_t)ticks.QuadPart;
}