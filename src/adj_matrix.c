
#include <stdlib.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "headers/deftypes.h"
#include "headers/math_utils.h"
#include "headers/adj_matrix.h"

#include "tests/headers/test_utils.h"

struct adj_matrix {
	uint node_count;
	double couplings[];
};

adj_matrix adj_matrix_create(uint node_count)
{
	adj_matrix result = malloc((sizeof *result) + (sizeof *(result->couplings)) * node_count * node_count);
	
	result->node_count = node_count;
	memset(result->couplings, 0, (sizeof *(result->couplings)) * node_count * node_count);
	
	return result;
}

void adj_matrix_set_custom(adj_matrix am, double (*element_setter)(uint size, uint row, uint col))
{
	assert(am);
	assert("Need a valid element_setter function pointer." && element_setter != NULL);

	for (uint row = 0; row < am->node_count; row++) {
		for (uint col = 0; col < am->node_count; col++) {
			am->couplings[row * am->node_count + col] = element_setter(am->node_count, row, col);
		}
	}
}

double adj_matrix_get(adj_matrix am, uint row, uint col)
{
	assert(am);
	assert(row < am->node_count);
	assert(col < am->node_count);

	return am->couplings[row * am->node_count + col];
}

uint adj_matrix_get_node_count(adj_matrix am)
{
	assert(am);
	return am->node_count;
}

void adj_matrix_destroy(adj_matrix *am)
{
	assert(am);
	assert(*am);

	free(*am);
	*am = NULL;
}
