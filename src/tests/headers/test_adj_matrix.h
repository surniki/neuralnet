
#ifndef TEST_ADJ_MATRIX_H
#define TEST_ADJ_MATRIX_H

#include <stdbool.h>

bool test_adj_matrix_create_destroy(void);
bool test_adj_matrix_get(void);
bool test_adj_matrix_set_empty(void);
bool test_adj_matrix_set_complete(void);
bool test_adj_matrix_set_lattice(void);
bool test_adj_matrix_set_custom(void);

#endif
