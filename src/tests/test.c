
#include "headers/test_file_array.h"
#include "headers/test_math_utils.h"
#include "headers/test_adj_matrix.h"
#include "headers/test_utils.h"

static const struct test_entry entries[] = {
	test_entry(test_file_array_create_destroy),
	test_entry(test_wrap_around),
	test_entry(test_adj_matrix_create_destroy),
	test_entry(test_adj_matrix_get),
	test_entry(test_adj_matrix_set_empty),
	test_entry(test_adj_matrix_set_complete),
	test_entry(test_adj_matrix_set_lattice),
	test_entry(test_adj_matrix_set_custom),
	null_entry
};

int main(void)
{
	return test_main(entries);
}
