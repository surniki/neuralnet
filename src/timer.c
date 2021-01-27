
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "headers/timer.h"

#include "tests/headers/test_utils.h"

struct timer {
	double start, previous;
};

static double current_time()
{
	return clock() / (double) CLOCKS_PER_SEC;
}

timer timer_begin()
{
	timer result = malloc(sizeof *result);
	result->start = result->previous = current_time();
	return result;
}

double timer_total_get(timer t)
{
	assert(t);
	return current_time() - t->start;
}

void timer_print(timer t, double time_interval, const char *fmt, ...)
{
	assert(t);
	if (current_time() - t->previous >= time_interval) {
		va_list vl;
		va_start(vl, fmt);
		vprintf(fmt, vl);
		va_end(vl);
		t->previous = current_time();
	}
}

void timer_end(timer *t, const char *fmt, ...)
{
	assert(t);
	assert(*t);

	if (fmt != NULL) {
		va_list vl;
		va_start(vl, fmt);
		vprintf(fmt, vl);
		va_end(vl);
	}

	free(*t);
	*t = NULL;
}
