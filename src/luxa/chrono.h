#pragma once

#include <luxa/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lx_highres_clock {
	double cpu_tick_delta;
}lx_highres_clock_t;

void lx_highres_clock_create(lx_highres_clock_t *clock);

int64_t lx_highres_clock_now();

static LX_INLINE double lx_highres_clock_milliseconds(lx_highres_clock_t *clock, int64_t ticks)
{
	return ticks * clock->cpu_tick_delta * 1000.0;
}

static LX_INLINE double lx_highres_clock_microseconds(lx_highres_clock_t *clock, int64_t ticks)
{
	return ticks * clock->cpu_tick_delta * 1000000.0;
}

#ifdef __cplusplus
}
#endif
