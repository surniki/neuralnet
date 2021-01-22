
#ifndef ADJ_MATRIX_H
#define ADJ_MATRIX_H

#include "deftypes.h"

struct adj_matrix;
typedef struct adj_matrix *adj_matrix;

adj_matrix adj_matrix_create(uint node_count);
void adj_matrix_set_complete(adj_matrix am, double cf);
void adj_matrix_set_lattice(adj_matrix am, uint width, uint height, double cf);
void adj_matrix_set_empty(adj_matrix am);
void adj_matrix_set_custom(adj_matrix am, double (*element_setter)(uint size, uint row, uint col));
double adj_matrix_get(adj_matrix am, uint row, uint col);
void adj_matrix_destroy(adj_matrix *am);

#endif
