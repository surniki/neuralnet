
#include "headers/test_file_array.h"
#include "../headers/file_array.h"
#include "headers/test_utils.h"

bool test_file_array_create_destroy()
{
	size_t prev_allocs = current_number_of_allocations();
	file_array fa = file_array_create(0, "test");
	bool test_1 = fa != NULL;
	file_array_destroy(&fa);
	bool test_2 = fa == NULL;
	bool test_3 = prev_allocs == current_number_of_allocations();

	return test_1 && test_2 && test_3;
}
