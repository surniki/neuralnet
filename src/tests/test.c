
#include "headers/test_file_table.h"
#include "headers/test_math_utils.h"
#include "headers/test_timer.h"
#include "headers/test_utils.h"
#include "headers/test_temp_memory.h"

static const struct test_entry entries[] = {
	test_entry(test_file_table_create_destroy),
	test_entry(test_temp_malloc),
	test_entry(test_temp_free),
	test_entry(test_math_utils_wrap_around),
	test_entry(test_math_utils_equal_within_tolerance),
	test_entry(test_math_utils_lattice_indices),
	test_entry(test_math_utils_rk4_integrate),
	test_entry(test_timer_begin_end),
	test_entry(test_timer_total_get),
	null_entry
};

int main(void)
{
	return test_main(entries);
}
