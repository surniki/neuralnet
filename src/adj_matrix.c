
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

void adj_matrix_set_complete(adj_matrix am, double cf)
{
	assert(am);
	for (uint row = 0; row < am->node_count; row++) {
		for (uint col = 0; col < am->node_count; col++) {
			if (row == col) {
				am->couplings[row * am->node_count + col] = 0.0;
			}
			else {
				am->couplings[row * am->node_count + col] = cf;
			}
		}
	}
}

void adj_matrix_set_empty(adj_matrix am)
{
	assert(am);
	adj_matrix_set_complete(am, 0.0);
}

void adj_matrix_set_lattice(adj_matrix am, uint width, uint height, double cf)
{
	assert(am);
	assert("Invalid lattice dimensions given." && am->node_count == width * height);
	assert("Width must be greater than 1." && width > 1);
	assert("Height must be greater than 1." && height > 1);

	adj_matrix_set_empty(am);

	uint left, right, up, down;
	for (int row = 0; row < am->node_count; row++) {
		left = (uint)wrap_around(row - 1, width * (int)(row / width), (1 + (int)(row / width)) * width - 1);
		right = (uint)wrap_around(row + 1, width * (int)(row / width), (1 + (int)(row / width)) * width - 1);
		up = (uint)wrap_around(row - width, 0, width * height - 1);
		down = (uint)wrap_around(row + width, 0, width * height - 1);
		am->couplings[row * am->node_count + left] = cf;
		am->couplings[row * am->node_count + right] = cf;
		am->couplings[row * am->node_count + up] = cf;
		am->couplings[row * am->node_count + down] = cf;
	}
}

void adj_matrix_set_custom(adj_matrix am, double (*element_setter)(uint size, uint row, uint col))
{
	assert("Need a valid element_setter function pointer." && element_setter != NULL);

	adj_matrix_set_empty(am);

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
