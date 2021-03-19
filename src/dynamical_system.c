
#include "headers/dynamical_system.h"
#include <stdlib.h>
#include <assert.h>

#include "tests/headers/test_utils.h"

struct dynamical_system {
	double time;
	uint system_size;
	uint element_size;
	void *(*parameter_callback)(dynamical_system ds, uint index);
	uint (*coupling_callback)(dynamical_system ds, uint first_index);
	double (**derivatives)(dynamical_system ds, uint system);
	struct edge *edge_pool;
	double elements[];
};

dynamical_system dynamical_system_create(uint system_size, uint element_size,
					 void *(*parameter_callback)(dynamical_system ds,
								    uint index),
					 uint (*coupling_callback)(dynamical_system ds,
								   uint first_index),
					 void (*initial_values_callback)(uint index,
									 uint size,
									 double *system_values),
					 double (**derivatives)(dynamical_system ds,
								uint system))
{
	dynamical_system result = malloc((sizeof *result) +
					 (sizeof *result->elements) * system_size * element_size);
	if (!result)
		return NULL;

	result->time = 0.0;
	result->system_size = system_size;
	result->element_size = element_size;
	result->parameter_callback = parameter_callback;
	result->coupling_callback = coupling_callback;
	result->derivatives = derivatives;
	result->edge_pool = malloc((sizeof *(result->edge_pool)) * system_size - 1);

	for (uint row = 0; row < system_size; row++) {
		initial_values_callback(row, element_size, &result->elements[row * element_size]);
	}

	return result;
}

void dynamical_system_destroy(dynamical_system *ds)
{
	free((*ds)->edge_pool);
	free(*ds);
	*ds = NULL;
}

void dynamical_system_increment_time(dynamical_system ds, double delta_t)
{
	ds->time += delta_t;
}

void dynamical_system_set_value(dynamical_system ds, uint row, uint column, double value)
{
	assert("Position described by row and column must be within the bounds of the system."
	       && row * ds->element_size + column < ds->system_size * ds->element_size);

	ds->elements[row * ds->element_size + column] = value;
}

void dynamical_system_increment_value(dynamical_system ds, uint row, uint column, double delta)
{
	assert("Position described by row and column must be within the bounds of the system."
	       && row * ds->element_size + column < ds->system_size * ds->element_size);

	ds->elements[row * ds->element_size + column] += delta;
}

double dynamical_system_get_value(dynamical_system ds, uint row, uint column)
{
	assert("Position described by row and column must be within the bounds of the system."
	       && row * ds->element_size + column < ds->system_size * ds->element_size);

	return ds->elements[row * ds->element_size + column];
}

double dynamical_system_get_time(dynamical_system ds)
{
	return ds->time;
}

uint dynamical_system_get_system_size(dynamical_system ds)
{
	return ds->system_size;
}

uint dynamical_system_get_element_size(dynamical_system ds)
{
	return ds->element_size;
}

double (**dynamical_system_get_derivatives(dynamical_system ds))(dynamical_system, uint)
{
	return ds->derivatives;
}

void *dynamical_system_get_parameters(dynamical_system ds, uint index)
{
	return ds->parameter_callback(ds, index);
}

uint dynamical_system_get_coupling(dynamical_system ds, uint first_index)
{
	return ds->coupling_callback(ds, first_index);
}

struct edge *dynamical_system_get_edge_pool(dynamical_system ds)
{
	return ds->edge_pool;
}

uint dynamical_system_get_edge_pool_size(dynamical_system ds)
{
	return ds->system_size - 1;
}
