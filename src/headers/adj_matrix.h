
#ifndef ADJ_MATRIX_H
#define ADJ_MATRIX_H

#include "deftypes.h"

struct adj_matrix;
typedef struct adj_matrix *adj_matrix;

adj_matrix adj_matrix_create(uint node_count);
void adj_matrix_set_custom(adj_matrix am, double (*element_setter)(uint size, uint row, uint col));
double adj_matrix_get(adj_matrix am, uint row, uint col);
uint adj_matrix_get_node_count(adj_matrix am);
void adj_matrix_destroy(adj_matrix *am);

#endif
