
#ifndef DYNAMICAL_SYSTEM_H
#define DYNAMICAL_SYSTEM_H

#include "deftypes.h"

struct dynamical_system;
typedef struct dynamical_system *dynamical_system;

struct edge {
	uint index;
	double value;
};

dynamical_system dynamical_system_create(uint system_size, uint grid_width, uint grid_height, uint element_size,
					 void *(*parameter_callback)(dynamical_system ds,
								    uint index),
					 uint (*coupling_callback)(dynamical_system ds,
								   uint first_index),
					 void (*initial_values_callback)(uint index,
									 uint size,
									 double *system_values),
					 double (**derivatives)(dynamical_system ds,
								uint system));
void dynamical_system_increment_time(dynamical_system ds, double delta_t);
void dynamical_system_set_value(dynamical_system ds, uint row, uint column, double value);
void dynamical_system_increment_value(dynamical_system ds, uint row, uint column, double delta);
double dynamical_system_get_value(dynamical_system ds, uint row, uint column);
double dynamical_system_get_time(dynamical_system ds);
uint dynamical_system_get_system_size(dynamical_system ds);
uint dynamical_system_get_element_size(dynamical_system ds);
double (**dynamical_system_get_derivatives(dynamical_system ds))(dynamical_system ds, uint system);
void *dynamical_system_get_parameters(dynamical_system ds, uint index);
uint dynamical_system_get_coupling(dynamical_system ds, uint first_index);
void dynamical_system_destroy(dynamical_system *ds);
struct edge *dynamical_system_get_edge_pool(dynamical_system ds);
uint dynamical_system_get_edge_pool_size(dynamical_system ds);
uint dynamical_system_get_grid_width(dynamical_system ds);
uint dynamical_system_get_grid_height(dynamical_system ds);

#endif
