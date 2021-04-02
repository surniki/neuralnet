
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

struct fitzhugh_nagumo_profile {
	double I_ext, a, b, tau;
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
	uint edges_found;
	if ((edges_found = dynamical_system_get_coupling(ds, index))) {
		const struct edge *edges = dynamical_system_get_edge_pool(ds);
		for (uint i = 0; i < edges_found; i++) {
			double coupled_V = dynamical_system_get_value(ds, edges[i].index, 0);
			I_coupling += edges[i].value * (V - coupled_V);
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

double fitzhugh_nagumo_dv_wrt_dt(dynamical_system ds, uint index)
{
	assert("Given index must be a valid number in the range [0, count)."
	       && index >= 0
	       && index < dynamical_system_get_system_size(ds));

	const struct fitzhugh_nagumo_profile *nrn = dynamical_system_get_parameters(ds, index);
	double v = dynamical_system_get_value(ds, index, 0);
	double w = dynamical_system_get_value(ds, index, 1);

	double I_coupling = 0.0;
	uint edges_found;
	if ((edges_found = dynamical_system_get_coupling(ds, index))) {
		const struct edge *edges = dynamical_system_get_edge_pool(ds);
		for (uint i = 0; i < edges_found; i++) {
			double coupled_v = dynamical_system_get_value(ds, edges[i].index, 0);
			I_coupling += edges[i].value * (v - coupled_v);
		}
	}
	
	return v - (pow(v, 3) / 3) - w + nrn->I_ext - I_coupling;
}

double fitzhugh_nagumo_dw_wrt_dt(dynamical_system ds, uint index)
{
	assert("Given index must be a valid number in the range [0, count)."
	       && index >= 0
	       && index < dynamical_system_get_system_size(ds));

	const struct fitzhugh_nagumo_profile *nrn = dynamical_system_get_parameters(ds, index);
	double v = dynamical_system_get_value(ds, index, 0);
	double w = dynamical_system_get_value(ds, index, 1);

	return (v + nrn->a - nrn->b * w) / nrn->tau;
}


struct dynamical_model fitzhugh_nagumo_model = (struct dynamical_model) {
	.derivatives = (double (*[])(dynamical_system ds, uint index)) {
		&fitzhugh_nagumo_dv_wrt_dt,
		&fitzhugh_nagumo_dw_wrt_dt
	},
	.number_of_variables = 2
};

static struct fitzhugh_nagumo_profile firing_profile = {
	.I_ext = 0.8,
	.a = 0.5,
	.b = 0.7,
	.tau = 1
};

static struct fitzhugh_nagumo_profile resting_profile = {
	.I_ext = 0.4,
	.a = 0.5,
	.b = 0.7,
	.tau = 1.0
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

void *fitzhugh_nagumo_parameter_callback_single_center(dynamical_system ds, uint index)
{
	uint width = dynamical_system_get_grid_width(ds);
	uint height = dynamical_system_get_grid_height(ds);

	uint center = width * (height / 2) + (width / 2);

	if (index == center) {
		return &firing_profile;
	}

	return &resting_profile;
}

void *huber_braun_parameter_callback_double_center(dynamical_system ds, uint index)
{
	uint width = dynamical_system_get_grid_width(ds);
	uint height = dynamical_system_get_grid_height(ds);

	uint center = width * (height / 2) + (width / 2);
	uint offset = width / 4;

	if (index == center + offset || index == center - offset) {
		return &tonic_profile;
	}

	return &bursting_profile;
}

void *huber_braun_parameter_callback_single_center(dynamical_system ds, uint index)
{
	uint width = dynamical_system_get_grid_width(ds);
	uint height = dynamical_system_get_grid_height(ds);

	uint center = width * (height / 2) + (width / 2);

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

uint coupling_callback_empty(dynamical_system ds, uint first_index)
{
	return 0; 
}

uint coupling_callback_complete(dynamical_system ds, uint first_index)
{
	struct edge *edge_pool = dynamical_system_get_edge_pool(ds);
	uint edge_pool_size = dynamical_system_get_edge_pool_size(ds);

	for (uint i = 0; i < edge_pool_size; i++) {
		edge_pool[i].index = (i >= first_index) ? i + 1 : i;
		edge_pool[i].value = coupled();
	}

	return edge_pool_size;
}

uint coupling_callback_lattice(dynamical_system ds, uint first_index)
{
	assert(dynamical_system_get_edge_pool_size(ds) >= 4);

	struct edge *edge_pool = dynamical_system_get_edge_pool(ds);
	uint width = dynamical_system_get_grid_width(ds);
	uint height = dynamical_system_get_grid_height(ds);

	math_utils_lattice_indices(first_index, width, height,
				   &(edge_pool[0].index),
				   &(edge_pool[1].index),
				   &(edge_pool[2].index),
				   &(edge_pool[3].index));

	edge_pool[0].value = coupled();
	edge_pool[1].value = coupled();
	edge_pool[2].value = coupled();
	edge_pool[3].value = coupled();

	return 4;
}

uint coupling_callback_line(dynamical_system ds, uint first_index)
{
	assert(dynamical_system_get_edge_pool_size(ds) >= 2);
	struct edge *edge_pool = dynamical_system_get_edge_pool(ds);
	uint width = dynamical_system_get_grid_width(ds);
	uint height = dynamical_system_get_grid_height(ds);
	
	int left = first_index - 1;
	int right = first_index + 1;
	uint size = dynamical_system_get_system_size(ds);

	edge_pool[0].index = left >= 0 ? left : size - 1;
	edge_pool[0].value = coupled();
	edge_pool[1].index = right < size ? right : 0;
	edge_pool[1].value = coupled();

	return 2;
}

uint coupling_callback_lattice_nowrap(dynamical_system ds, uint first_index)
{
	assert(dynamical_system_get_edge_pool_size(ds) >= 4);

	struct edge *edge_pool = dynamical_system_get_edge_pool(ds);
	uint width = dynamical_system_get_grid_width(ds);
	uint height = dynamical_system_get_grid_height(ds);

	int left = first_index - 1;
	int right = first_index + 1;
	int top = first_index - width;
	int bottom = first_index + width;

	int left_border = (first_index / width) * width;
	int right_border = left_border + width - 1;
	int top_border = 0;
	int bottom_border = dynamical_system_get_system_size(ds) - 1;	

	uint edge_count = 0;
	if (left >= left_border) {
		edge_pool[edge_count].index = left;
		edge_pool[edge_count].value = coupled();
		edge_count++;
	}
	if (right <= right_border) {
		edge_pool[edge_count].index = right;
		edge_pool[edge_count].value = coupled();
		edge_count++;
	}
	if (top >= top_border) {
		edge_pool[edge_count].index = top;
		edge_pool[edge_count].value = coupled();
		edge_count++;
	}
	if (bottom <= bottom_border) {
		edge_pool[edge_count].index = bottom;
		edge_pool[edge_count].value = coupled();
		edge_count++;
	}

	return edge_count;
}

uint coupling_callback_all_neighbors(dynamical_system ds, uint first_index)
{
	assert(dynamical_system_get_edge_pool_size(ds) >= 8);

	struct edge *edge_pool = dynamical_system_get_edge_pool(ds);
	uint width = dynamical_system_get_grid_width(ds);
	uint height = dynamical_system_get_grid_height(ds);

	math_utils_all_neighbors_indices(first_index, width, height,
					 &(edge_pool[0].index),
					 &(edge_pool[1].index),
					 &(edge_pool[2].index),
					 &(edge_pool[3].index),
					 &(edge_pool[4].index),
					 &(edge_pool[5].index),
					 &(edge_pool[6].index),
					 &(edge_pool[7].index));

	edge_pool[0].value = coupled();
	edge_pool[1].value = coupled();
	edge_pool[2].value = coupled();
	edge_pool[3].value = coupled();
	edge_pool[4].value = coupled();
	edge_pool[5].value = coupled();
	edge_pool[6].value = coupled();
	edge_pool[7].value = coupled();

	return 8;
}
