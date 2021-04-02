
#ifndef NEURON_CONFIG_H
#define NEURON_CONFIG_H

extern struct dynamical_model huber_braun_model;
extern struct dynamical_model fitzhugh_nagumo_model;

struct dynamical_model {
	double (**derivatives)(dynamical_system ds, uint index);
	uint number_of_variables;
};

void neuron_config_coupling_is_random_set(bool value, double lowest, double highest);
void neuron_config_coupling_constant_set(double value);

void initial_values_callback_zero(uint index, uint size, double *elements);

void *huber_braun_parameter_callback_double_center(dynamical_system ds, uint index);
void *huber_braun_parameter_callback_single_center(dynamical_system ds, uint index);
void *huber_braun_parameter_callback_bursting(dynamical_system ds, uint index);
void *huber_braun_parameter_callback_tonic(dynamical_system ds, uint index);

void *fitzhugh_nagumo_parameter_callback_single_center(dynamical_system ds, uint index);

uint coupling_callback_empty(dynamical_system ds, uint first_index);
uint coupling_callback_lattice(dynamical_system ds, uint first_index);
uint coupling_callback_complete(dynamical_system ds, uint first_index);
uint coupling_callback_lattice_nowrap(dynamical_system ds, uint first_index);
uint coupling_callback_all_neighbors(dynamical_system ds, uint first_index);
uint coupling_callback_line(dynamical_system ds, uint first_index);

#endif
