
#include <stdbool.h>
#include <math.h>
#include "headers/neuron.h"
#include "headers/math_utils.h"
#include "headers/neuron_config.h"

bool coupling_constant_is_random;
double lowest_random_value;
double highest_random_value;
double coupling_constant;

void neuron_config_coupling_is_random_set(bool value, double lowest, double highest)
{
	coupling_constant_is_random = value;
	lowest_random_value = lowest;
	highest_random_value = highest;
}

void neuron_config_coupling_constant_set(double value)
{
	coupling_constant_is_random = false;
	coupling_constant = value;
}

static double coupled()
{
	if (coupling_constant_is_random) {
		return math_utils_random_number(lowest_random_value, highest_random_value);
	}

	return coupling_constant;
}

static double not_coupled()
{
	return 0.0;
}

bool is_neuron_init_entry_empty(struct neuron_init_entry e)
{
	return e.name == (void *)0 && e.desc == (void *)0 && e.callback == (void *)0;
}

bool is_adj_matrix_init_entry_empty(struct adj_matrix_init_entry e)
{
	return e.name == (void *)0 && e.desc == (void *)0 && e.callback == (void *)0;
}

static struct neuron_profile tonic_profile = {
	.I_inj = 0.0,     /* checked */
	.C = 1.0,         /* checked */
	.g_leak = 0.1,    /* checked */
	.V_leak = -60.0,  /* checked */
	.rho = 0.607,     /* checked */
	.g_Na = 1.5,      /* checked, changed from 1.5 -> 1.0 */
	.V_Na = 50.0,     /* checked */
	.g_K = 2.0,       /* checked */
	.V_K = -90.0,     /* checked */
	.g_sd = 0.25,     /* checked */
	.V_sd = 50.0,     /* checked */
	.g_sr = 0.25,     /* checked, in tonic range */
	.V_sr = -90.0,    /* checked */
	.s_Na = 0.25,     /* checked */
	.V_0Na = -25.0,   /* checked */
	.phi = 0.124,     /* checked */
	.tau_K = 2.0,     /* checked */
	.tau_sd = 10.0,   /* checked */
	.tau_sr = 20.0,   /* checked */
	.v_acc = 0.012,   /* checked, changed from 0.17 -> 0.012 */
	.v_dep = 0.17,    /* checked, changed from 0.012 -> 0.17  */
	.s_K = 0.25,      /* checked */
	.V_0K = -25.0,    /* checked */
	.s_sd = 0.09,     /* checked */
	.V_0sd = -40.0    /* checked */
};

static struct neuron_profile bursting_profile = {
	.I_inj = 1.0,     /* checked, should have frequency ~1Hz */
	.C = 1.0,         /* checked */
	.g_leak = 0.1,    /* checked */
	.V_leak = -60.0,  /* checked */
	.rho = 0.607,     /* checked */
	.g_Na = 1.5,      /* checked */
	.V_Na = 50.0,     /* checked */
	.g_K = 2.0,       /* checked */
	.V_K = -90.0,     /* checked */
	.g_sd = 0.25,     /* checked */
	.V_sd = 50.0,     /* checked */
	.g_sr = 0.35,     /* checked, in bursting range */
	.V_sr = -90.0,    /* checked */
	.s_Na = 0.25,     /* checked */
	.V_0Na = -25.0,   /* checked */
	.phi = 0.124,     /* checked */
	.tau_K = 2.0,     /* checked */
	.tau_sd = 10.0,   /* checked */
	.tau_sr = 20.0,   /* checked */
	.v_acc = 0.012,   /* checked, changed from 0.17 -> 0.012 */
	.v_dep = 0.17,    /* checked, changed from 0.012 -> 0.17  */
	.s_K = 0.25,      /* checked */
	.V_0K = -25.0,    /* checked */
	.s_sd = 0.09,     /* checked */
	.V_0sd = -40.0    /* checked */
};

void neuron_init_callback_double_center(uint i, uint count,
					double *V, double *a_K, double *a_sd, double *a_sr,
					struct neuron_profile **np)
{
	uint side_length = sqrt(count);
	uint center = side_length * (side_length / 2) + (side_length / 2);
	uint offset = side_length / 4;

	if (i == center + offset || i == center - offset)
		*np = &tonic_profile;
	else
		*np = &bursting_profile;
	
	*V    = 0.0;
	*a_K  = 0.0;
	*a_sd = 0.0;
	*a_sr = 0.0;
}

void neuron_init_callback_single_center(uint i, uint count,
					double *V, double *a_K, double *a_sd, double *a_sr,
					struct neuron_profile **np)
{
	uint side_length = sqrt(count);
	uint center = side_length * (side_length / 2) + (side_length / 2);

	if (i == center)
		*np   = &tonic_profile;
	else
		*np   = &bursting_profile;

	*V    = 0.0;
	*a_K  = 0.0;
	*a_sd = 0.0;
	*a_sr = 0.0;
}

void neuron_init_callback_bursting(uint i, uint count,
				   double *V, double *a_K, double *a_sd, double *a_sr,
				   struct neuron_profile **np)
{
	*np   = &bursting_profile;
	*V    = 0.0;
	*a_K  = 0.0;
	*a_sd = 0.0;
	*a_sr = 0.0;
}

void neuron_init_callback_tonic(uint i, uint count,
				double *V, double *a_K, double *a_sd, double *a_sr,
				struct neuron_profile **np)
{
	*np   = &tonic_profile;
	*V    = 0.0;
	*a_K  = 0.0;
	*a_sd = 0.0;
	*a_sr = 0.0;
}

double adj_matrix_init_callback_empty(uint size, uint row, uint col)
{
	return not_coupled(); 
}

double adj_matrix_init_callback_lattice(uint size, uint row, uint col)
{
	uint top, right, bottom, left;
	uint side_length = sqrt(size);
	math_utils_lattice_indices(row, side_length, side_length, &top, &right, &bottom, &left);

	if (col == top || col == right || col == bottom || col == left) {
		return coupled();
	}

	return not_coupled();
}

double adj_matrix_init_callback_complete(uint size, uint row, uint col)
{
	if (row == col) {
		return not_coupled();
	}

	return coupled();
}
