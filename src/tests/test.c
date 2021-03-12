
#include "headers/test_file_table.h"
#include "headers/test_math_utils.h"
#include "headers/test_adj_matrix.h"
#include "headers/test_neuron.h"
#include "headers/test_timer.h"
#include "headers/test_utils.h"

static const struct test_entry entries[] = {
	test_entry(test_file_table_create_destroy),
	test_entry(test_math_utils_wrap_around),
	test_entry(test_math_utils_equal_within_tolerance),
	test_entry(test_math_utils_lattice_indices),
	test_entry(test_adj_matrix_create_destroy),
	test_entry(test_adj_matrix_get),
	test_entry(test_adj_matrix_set_empty),
	test_entry(test_adj_matrix_set_complete),
	test_entry(test_adj_matrix_set_lattice),
	test_entry(test_adj_matrix_set_custom),
	test_entry(test_timer_begin_end),
	test_entry(test_timer_total_get),
	test_entry(test_neural_network_create_destroy),
	test_entry(test_neural_network_integrate_single_tonic),
	test_entry(test_neural_network_integrate_single_bursting),
	test_entry(test_neural_network_integrate_3x3_uncoupled),
	test_entry(test_neural_network_integrate_3x3_coupled),
	test_entry(test_neural_network_integrate_single_tonic_backwards),
	test_entry(test_neural_network_integrate_3x3_coupled_backwards),
	null_entry
};

int main(void)
{
	return test_main(entries);
}
