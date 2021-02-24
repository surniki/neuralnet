
#include "headers/test_file_table.h"
#include "../headers/file_table.h"
#include "headers/test_utils.h"

bool test_file_table_create_destroy()
{
	size_t prev_allocs = current_number_of_allocations();
	file_table fa = file_table_create("test", 0, 0);
	bool test_1 = fa != NULL;
	file_table_destroy(&fa);
	bool test_2 = fa == NULL;
	bool test_3 = prev_allocs == current_number_of_allocations();

	return test_1 && test_2 && test_3;
}
