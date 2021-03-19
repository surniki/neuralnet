
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include "headers/math_utils.h"
#include "headers/neuron_config.h"

#include "tests/headers/test_utils.h"

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

struct huber_braun_profile  {
	double I_inj, C, g_leak, V_leak, rho, g_Na, V_Na, g_K, V_K, g_sd, V_sd,
		g_sr, V_sr, s_Na, V_0Na, phi, tau_K, tau_sd, tau_sr, v_acc, v_dep,
		s_K, V_0K, s_sd, V_0sd;
};

double huber_braun_dV_wrt_dt(dynamical_system ds, uint index)
{
	assert("Given index must be a valid number in the range [0, count)."
	       && index >= 0
	       && index < dynamical_system_get_system_size(ds));

	const struct huber_braun_profile *nrn = dynamical_system_get_parameters(ds, index);
	double V    = dynamical_system_get_value(ds, index, 0);
	double a_K  = dynamical_system_get_value(ds, index, 1);
	double a_sd = dynamical_system_get_value(ds, index, 2);
	double a_sr = dynamical_system_get_value(ds, index, 3);

	double I_coupling = 0.0;
	for (uint col = 0; col < dynamical_system_get_system_size(ds); col++) {
		double g_c = dynamical_system_get_coupling(ds, index, col);
		if (g_c != 0.0) {	
			double coupled_V = dynamical_system_get_value(ds, col, 0);
			I_coupling += g_c * (V - coupled_V);
		}
	}
	
	const double I_leak = nrn->g_leak * (V - nrn->V_leak);
	const double a_Na   = 1.0 / (1.0 + exp(-nrn->s_Na * (V - nrn->V_0Na)));

	const double I_Na   = nrn->rho * nrn->g_Na * a_Na * (V - nrn->V_Na);
	const double I_K    = nrn->rho * nrn->g_K  * a_K  * (V - nrn->V_K);
	const double I_sd   = nrn->rho * nrn->g_sd * a_sd * (V - nrn->V_sd);
	const double I_sr   = nrn->rho * nrn->g_sr * a_sr * (V - nrn->V_sr);

	return -(I_leak + I_Na + I_K + I_sd + I_sr + nrn->I_inj + I_coupling) / nrn->C;
}

double huber_braun_da_K_wrt_dt(dynamical_system ds, uint index)
{
	assert("Given index must be a valid number in the range [0, count)."
	       && index >= 0
	       && index < dynamical_system_get_system_size(ds));

	const struct huber_braun_profile *nrn = dynamical_system_get_parameters(ds, index);
	double V    = dynamical_system_get_value(ds, index, 0);
	double a_K  = dynamical_system_get_value(ds, index, 1);
	double a_sd = dynamical_system_get_value(ds, index, 2);
	double a_sr = dynamical_system_get_value(ds, index, 3);
	
	const double a_K_inf = 1.0 / (1.0 + exp(-nrn->s_K * (V - nrn->V_0K))); /* check */
	return (nrn->phi / nrn->tau_K) * (a_K_inf - a_K); /* check */
}

double huber_braun_da_sd_wrt_dt(dynamical_system ds, uint index)
{
	assert("Given index must be a valid number in the range [0, count)."
	       && index >= 0
	       && index < dynamical_system_get_system_size(ds));

	const struct huber_braun_profile *nrn = dynamical_system_get_parameters(ds, index);
	double V    = dynamical_system_get_value(ds, index, 0);
	double a_K  = dynamical_system_get_value(ds, index, 1);
	double a_sd = dynamical_system_get_value(ds, index, 2);
	double a_sr = dynamical_system_get_value(ds, index, 3);       

	const double a_sd_inf = 1.0 / (1.0 + exp(-nrn->s_sd * (V - nrn->V_0sd)));
	return (nrn->phi / nrn->tau_sd) * (a_sd_inf - a_sd);
}

double huber_braun_da_sr_wrt_dt(dynamical_system ds, uint index)
{
	assert("Given index must be a valid number in the range [0, count)."
	       && index >= 0
	       && index < dynamical_system_get_system_size(ds));

	const struct huber_braun_profile *nrn = dynamical_system_get_parameters(ds, index);
	double V    = dynamical_system_get_value(ds, index, 0);
	double a_K  = dynamical_system_get_value(ds, index, 1);
	double a_sd = dynamical_system_get_value(ds, index, 2);
	double a_sr = dynamical_system_get_value(ds, index, 3);       

	const double I_sd = nrn->rho * nrn->g_sd * a_sd * (V - nrn->V_sd);
	return -(nrn->phi / nrn->tau_sr) * (nrn->v_acc * I_sd + nrn->v_dep * a_sr);
}

struct dynamical_model huber_braun_model = (struct dynamical_model) {
	.derivatives = (double (*[])(dynamical_system ds, uint index)) {
		&huber_braun_dV_wrt_dt,
		&huber_braun_da_K_wrt_dt,
		&huber_braun_da_sd_wrt_dt,
		&huber_braun_da_sr_wrt_dt
	},
	.number_of_variables = 4
};
	
static struct huber_braun_profile tonic_profile = {
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

static struct huber_braun_profile bursting_profile = {
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

void initial_values_callback_zero(uint index, uint size, double *elements)
{
	for (uint i = 0; i < size; i++) {
		elements[i] = 0.0;
	}
}

void *huber_braun_parameter_callback_double_center(dynamical_system ds, uint index)
{
	uint side_length = sqrt(dynamical_system_get_system_size(ds));
	uint center = side_length * (side_length / 2) + (side_length / 2);
	uint offset = side_length / 4;

	if (index == center + offset || index == center - offset) {
		return &tonic_profile;
	}

	return &bursting_profile;
}

void *huber_braun_parameter_callback_single_center(dynamical_system ds, uint index)
{
	uint side_length = sqrt(dynamical_system_get_system_size(ds));
	uint center = side_length * (side_length / 2) + (side_length / 2);

	if (index == center) {
		return &tonic_profile;
	}

	return &bursting_profile;
}

void *huber_braun_parameter_callback_bursting(dynamical_system ds, uint index)
{
	return &bursting_profile;
}

void *huber_braun_parameter_callback_tonic(dynamical_system ds, uint index)
{
	return &tonic_profile;
}

double coupling_callback_empty(dynamical_system ds, uint first_index, uint second_index)
{
	return not_coupled(); 
}

double coupling_callback_lattice(dynamical_system ds, uint first_index, uint second_index)
{
	uint top, right, bottom, left;
	uint side_length = sqrt(dynamical_system_get_system_size(ds));
	math_utils_lattice_indices(first_index, side_length, side_length,
				   &top, &right, &bottom, &left);

	if (second_index == top    ||
	    second_index == right  ||
	    second_index == bottom ||
	    second_index == left) {
		return coupled();
	}

	return not_coupled();
}

double coupling_callback_complete(dynamical_system ds, uint first_index, uint second_index)
{
	if (first_index == second_index) {
		return not_coupled();
	}

	return coupled();
}
