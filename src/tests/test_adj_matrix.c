
#include <stdlib.h>
#include "../headers/deftypes.h"
#include "headers/test_adj_matrix.h"
#include "../headers/adj_matrix.h"
#include "headers/test_utils.h"

bool test_adj_matrix_create_destroy(void)
{
	size_t prev_alloc = current_number_of_allocations();

	adj_matrix am = adj_matrix_create(10);
	bool test_1 = current_number_of_allocations() > prev_alloc;

	bool test_2 = is_assert_invoked(adj_matrix_destroy(NULL));

	adj_matrix_destroy(&am);
	bool test_3 = current_number_of_allocations() == prev_alloc;
	bool test_4 = am == NULL;
	
	return test_1 && test_2 && test_3;
}

bool test_adj_matrix_get(void)
{
	adj_matrix am = adj_matrix_create(25);
	bool test_1 = !is_assert_invoked(adj_matrix_get(am, 0, 0));
	bool test_2 = !is_assert_invoked(adj_matrix_get(am, 24, 24));
	bool test_3 = is_assert_invoked(adj_matrix_get(am, 10, 25));
	bool test_4 = is_assert_invoked(adj_matrix_get(am, 25, 10));
	bool test_5 = !is_assert_invoked(adj_matrix_get(am, 10, 11));
	adj_matrix_destroy(&am);

	return test_1 && test_2 && test_3 && test_4 && test_5;
}

bool test_adj_matrix_set_empty(void)
{
	adj_matrix am = adj_matrix_create(5);

	bool test_1 = is_assert_invoked(adj_matrix_set_empty(NULL));

	adj_matrix_set_empty(am);

	bool test_2 = true;
	for (uint row = 0; row < 5; row++) {
		for (uint col = 0; col < 5; col++) {
			test_1 = test_1 && adj_matrix_get(am, row, col) == 0.0;
		}
	}
	
	adj_matrix_destroy(&am);

	return test_1 && test_2;
}

bool test_adj_matrix_set_complete(void)
{
	bool test_1 = is_assert_invoked(adj_matrix_set_complete(NULL, 1.0));

	adj_matrix am = adj_matrix_create(20);

	adj_matrix_set_complete(am, 1.0);
	bool test_2 = true;
	for (uint row = 0; row < 20; row++) {
		for (uint col = 0; col < 20; col++) {
			test_2 = test_2 && adj_matrix_get(am, row, col) == 1.0;
		}
	}

	adj_matrix_set_complete(am, 0.5);
	bool test_3 = true;
	for (uint row = 0; row < 20; row++) {
		for (uint col = 0; col < 20; col++) {
			test_3 = test_3 && adj_matrix_get(am, row, col) == 0.5;
		}
	}

	adj_matrix_destroy(&am);

	return test_1 && test_2 && test_3;
}

bool test_adj_matrix_set_lattice(void)
{
	adj_matrix am = adj_matrix_create(4);
	bool test_1 = is_assert_invoked(adj_matrix_set_lattice(NULL, 5, 5, 1.0));
	bool test_2 = is_assert_invoked(adj_matrix_set_lattice(am, 5, 5, 1.0));
	bool test_3 = is_assert_invoked(adj_matrix_set_lattice(am, 0, 0, 1.0));

	adj_matrix_set_lattice(am, 2, 2, 1.0);
	bool test_4 = true;
	test_4 = test_4 && adj_matrix_get(am, 0, 0) == 0.0;
	test_4 = test_4 && adj_matrix_get(am, 0, 1) == 1.0;
	test_4 = test_4 && adj_matrix_get(am, 0, 2) == 1.0;
	test_4 = test_4 && adj_matrix_get(am, 0, 3) == 0.0;
	test_4 = test_4 && adj_matrix_get(am, 1, 0) == 1.0;
	test_4 = test_4 && adj_matrix_get(am, 1, 1) == 0.0;
	test_4 = test_4 && adj_matrix_get(am, 1, 2) == 0.0;
	test_4 = test_4 && adj_matrix_get(am, 1, 3) == 1.0;
	test_4 = test_4 && adj_matrix_get(am, 2, 0) == 1.0;
	test_4 = test_4 && adj_matrix_get(am, 2, 1) == 0.0;
	test_4 = test_4 && adj_matrix_get(am, 2, 2) == 0.0;
	test_4 = test_4 && adj_matrix_get(am, 2, 3) == 1.0;
	test_4 = test_4 && adj_matrix_get(am, 3, 0) == 0.0;
	test_4 = test_4 && adj_matrix_get(am, 3, 1) == 1.0;
	test_4 = test_4 && adj_matrix_get(am, 3, 2) == 1.0;
	test_4 = test_4 && adj_matrix_get(am, 3, 3) == 0.0;
	adj_matrix_destroy(&am);

	am = adj_matrix_create(6);
	adj_matrix_set_lattice(am, 2, 3, 1.0);
	bool test_5 = true;
	test_5 = test_5 && adj_matrix_get(am, 0, 0) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 0, 1) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 0, 2) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 0, 3) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 0, 4) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 0, 5) == 0.0;

	test_5 = test_5 && adj_matrix_get(am, 1, 0) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 1, 1) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 1, 2) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 1, 3) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 1, 4) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 1, 5) == 1.0;

	test_5 = test_5 && adj_matrix_get(am, 2, 0) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 2, 1) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 2, 2) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 2, 3) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 2, 4) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 2, 5) == 0.0;

	test_5 = test_5 && adj_matrix_get(am, 3, 0) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 3, 1) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 3, 2) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 3, 3) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 3, 4) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 3, 5) == 1.0;

	test_5 = test_5 && adj_matrix_get(am, 4, 0) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 4, 1) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 4, 2) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 4, 3) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 4, 4) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 4, 5) == 1.0;

	test_5 = test_5 && adj_matrix_get(am, 5, 0) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 5, 1) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 5, 2) == 0.0;
	test_5 = test_5 && adj_matrix_get(am, 5, 3) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 5, 4) == 1.0;
	test_5 = test_5 && adj_matrix_get(am, 5, 5) == 0.0;
	adj_matrix_destroy(&am);

	am = adj_matrix_create(9);
	adj_matrix_set_lattice(am, 3, 3, 1.0);
	bool test_6 = true;
	test_6 = test_6 && adj_matrix_get(am, 0, 0) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 0, 1) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 0, 2) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 0, 3) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 0, 4) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 0, 5) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 0, 6) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 0, 7) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 0, 8) == 0.0;

	test_6 = test_6 && adj_matrix_get(am, 1, 0) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 1, 1) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 1, 2) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 1, 3) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 1, 4) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 1, 5) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 1, 6) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 1, 7) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 1, 8) == 0.0;

	test_6 = test_6 && adj_matrix_get(am, 2, 0) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 2, 1) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 2, 2) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 2, 3) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 2, 4) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 2, 5) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 2, 6) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 2, 7) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 2, 8) == 1.0;

	test_6 = test_6 && adj_matrix_get(am, 3, 0) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 3, 1) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 3, 2) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 3, 3) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 3, 4) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 3, 5) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 3, 6) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 3, 7) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 3, 8) == 0.0;

	test_6 = test_6 && adj_matrix_get(am, 4, 0) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 4, 1) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 4, 2) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 4, 3) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 4, 4) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 4, 5) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 4, 6) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 4, 7) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 4, 8) == 0.0;

	test_6 = test_6 && adj_matrix_get(am, 5, 0) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 5, 1) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 5, 2) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 5, 3) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 5, 4) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 5, 5) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 5, 6) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 5, 7) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 5, 8) == 1.0;

	test_6 = test_6 && adj_matrix_get(am, 6, 0) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 6, 1) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 6, 2) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 6, 3) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 6, 4) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 6, 5) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 6, 6) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 6, 7) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 6, 8) == 1.0;

	test_6 = test_6 && adj_matrix_get(am, 7, 0) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 7, 1) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 7, 2) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 7, 3) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 7, 4) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 7, 5) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 7, 6) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 7, 7) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 7, 8) == 1.0;

	test_6 = test_6 && adj_matrix_get(am, 8, 0) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 8, 1) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 8, 2) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 8, 3) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 8, 4) == 0.0;
	test_6 = test_6 && adj_matrix_get(am, 8, 5) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 8, 6) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 8, 7) == 1.0;
	test_6 = test_6 && adj_matrix_get(am, 8, 8) == 0.0;

	adj_matrix_destroy(&am);

	return test_1 && test_2 && test_3 && test_4 && test_5 && test_6;
}


/* Set the (4, 3) element (in 1-based math indexing) to be 1.0, and
 * everything else to be 0.0.
 */
double test_custom_setter(uint size, uint row, uint col)
{
	return (row == 3 && col == 2) ? 1.0 : 0.0;
}

bool test_adj_matrix_set_custom(void)
{
	bool test_1 = true;
	adj_matrix am = adj_matrix_create(1);
	adj_matrix_set_custom(am, &test_custom_setter);
	for (int row = 0; row < 1; row++) {
		for (int col = 0; col < 1; col++) {
			test_1 = test_1 && adj_matrix_get(am, row, col) == 0.0;
		}
	}
	adj_matrix_destroy(&am);

	bool test_2 = true;
	am = adj_matrix_create(2);
	adj_matrix_set_custom(am, &test_custom_setter);
	for (int row = 0; row < 2; row++) {
		for (int col = 0; col < 2; col++) {
			test_2 = test_2 && adj_matrix_get(am, row, col) == 0.0;
		}
	}
	adj_matrix_destroy(&am);

	bool test_3 = true;
	am = adj_matrix_create(3);
	adj_matrix_set_custom(am, &test_custom_setter);
	for (int row = 0; row < 3; row++) {
		for (int col = 0; col < 3; col++) {
			test_3 = test_3 && adj_matrix_get(am, row, col) == 0.0;
		}
	}
	adj_matrix_destroy(&am);

	bool test_4 = true;
	am = adj_matrix_create(4);
	adj_matrix_set_custom(am, &test_custom_setter);
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			if (row == 3 && col == 2)
				test_4 = test_4 && adj_matrix_get(am, row, col) == 1.0;
			else
				test_4 = test_4 && adj_matrix_get(am, row, col) == 0.0;
		}
	}
	adj_matrix_destroy(&am);

	bool test_5 = true;
	am = adj_matrix_create(5);
	adj_matrix_set_custom(am, &test_custom_setter);
	for (int row = 0; row < 5; row++) {
		for (int col = 0; col < 5; col++) {
			if (row == 3 && col == 2)
				test_5 = test_5 && adj_matrix_get(am, row, col) == 1.0;
			else
				test_5 = test_5 && adj_matrix_get(am, row, col) == 0.0;
		}
	}
	adj_matrix_destroy(&am);

	return test_1 && test_2 && test_3 && test_4 && test_5;
}
