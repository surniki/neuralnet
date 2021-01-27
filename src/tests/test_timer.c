
#include <stdlib.h>
#include "headers/test_timer.h"
#include "../headers/timer.h"
#include "headers/test_utils.h"

bool test_timer_begin_end()
{
	bool test_1 = is_assert_invoked(timer_end(NULL, NULL));

	size_t allocs = current_number_of_allocations();
	timer t = timer_begin();
	timer_end(&t, NULL);

	bool test_2 = allocs == current_number_of_allocations();

	return test_1 && test_2;
}

bool test_timer_total_get()
{
	timer t = timer_begin();

	bool test_1 = is_assert_invoked(timer_total_get(NULL));
	
	double prev = timer_total_get(t);
	bool test_2 = timer_total_get(t) >= prev;
	prev = timer_total_get(t);
	bool test_3 = timer_total_get(t) >= prev;
	prev = timer_total_get(t);
	bool test_4 = timer_total_get(t) >= prev;

	timer_end(&t, NULL);
	
	return test_1 && test_2 && test_3 && test_4;
}
