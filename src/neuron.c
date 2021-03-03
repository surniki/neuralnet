
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "headers/deftypes.h"
#include "headers/neuron.h"
#include "headers/file_table.h"
#include "headers/adj_matrix.h"

#include "tests/headers/test_utils.h"

struct neural_network {
	uint count;
	double time;
	adj_matrix adj_matrix;
	struct neuron_profile **profiles;
	double *V, *a_K, *a_sd, *a_sr;
	double *k1_V, *k1_a_K, *k1_a_sd, *k1_a_sr;
	double *k2_V, *k2_a_K, *k2_a_sd, *k2_a_sr;
	double *k3_V, *k3_a_K, *k3_a_sd, *k3_a_sr;
	double *k4_V, *k4_a_K, *k4_a_sd, *k4_a_sr;
};

static double neuron_dV_wrt_dt(neural_network ns, uint index, enum runge_kutta_4_k_context k_number)
{
	assert("Given index must be a valid number in the range [0, count)."
	       && index >= 0
	       && index < ns->count);

	const struct neuron_profile *nrn = ns->profiles[index];
	double V = ns->V[index];
	double a_K = ns->a_K[index];
	double a_sd = ns->a_sd[index];
	double a_sr = ns->a_sr[index];

	switch (k_number) {
	case RK4_K_1:
		break;
	case RK4_K_2:
		V    += ns->k1_V[index] / 2.0;
		a_K  += ns->k1_a_K[index] / 2.0;
		a_sd += ns->k1_a_sd[index] / 2.0;
		a_sr += ns->k1_a_sr[index] / 2.0;
		break;
	case RK4_K_3:
		V    += ns->k2_V[index] / 2.0;
		a_K  += ns->k2_a_K[index] / 2.0;
		a_sd += ns->k2_a_sd[index] / 2.0;
		a_sr += ns->k2_a_sr[index] / 2.0;
		break;
	case RK4_K_4:
		V    += ns->k3_V[index];
		a_K  += ns->k3_a_K[index];
		a_sd += ns->k3_a_sd[index];
		a_sr += ns->k3_a_sr[index];
		break;
	default:
		assert(!"Unsupported k_number used as argument.");
		break;
	}
	
	double I_coupling = 0.0;	
	for (uint col = 0; col < adj_matrix_get_node_count(ns->adj_matrix); col++) {
		double g_c = adj_matrix_get(ns->adj_matrix, index, col);
		if (g_c != 0.0) {	
			double coupled_V = ns->V[col];
			switch (k_number) {
			case RK4_K_1:
				break;
			case RK4_K_2:
				coupled_V += ns->k1_V[col] / 2.0;
				break;
			case RK4_K_3:
				coupled_V += ns->k2_V[col] / 2.0;
				break;
			case RK4_K_4:
				coupled_V += ns->k3_V[col];
				break;
			default:
				assert(!"Unsupported k_number used as argument.");
				break;
			}
			I_coupling += g_c * (V - coupled_V);
		}
	}

	const double I_leak = nrn->g_leak * (V - nrn->V_leak); /* check */
	const double a_Na   = 1.0 / (1.0 + exp(-nrn->s_Na * (V - nrn->V_0Na))); /* check */

	const double I_Na   = nrn->rho * nrn->g_Na * a_Na * (V - nrn->V_Na); /* check */
	const double I_K    = nrn->rho * nrn->g_K  * a_K  * (V - nrn->V_K); /* check */
	const double I_sd   = nrn->rho * nrn->g_sd * a_sd * (V - nrn->V_sd); /* check */
	const double I_sr   = nrn->rho * nrn->g_sr * a_sr * (V - nrn->V_sr); /* check */

	return -(I_leak + I_Na + I_K + I_sd + I_sr + nrn->I_inj + I_coupling) / nrn->C; /* check */
}

static double neuron_da_K_wrt_dt(neural_network ns, uint index, enum runge_kutta_4_k_context k_number)
{
	assert("Given index must be a valid number in the range [0, count)."
	       && index >= 0
	       && index < ns->count);

	const struct neuron_profile *nrn = ns->profiles[index];
	double V = ns->V[index];
	double a_K = ns->a_K[index];
	double a_sd = ns->a_sd[index];
	double a_sr = ns->a_sr[index];

	switch (k_number) {
	case RK4_K_1:
		break;
	case RK4_K_2:
		V    += ns->k1_V[index] / 2.0;
		a_K  += ns->k1_a_K[index] / 2.0;
		a_sd += ns->k1_a_sd[index] / 2.0;
		a_sr += ns->k1_a_sr[index] / 2.0;
		break;
	case RK4_K_3:
		V    += ns->k2_V[index] / 2.0;
		a_K  += ns->k2_a_K[index] / 2.0;
		a_sd += ns->k2_a_sd[index] / 2.0;
		a_sr += ns->k2_a_sr[index] / 2.0;
		break;
	case RK4_K_4:
		V    += ns->k3_V[index];
		a_K  += ns->k3_a_K[index];
		a_sd += ns->k3_a_sd[index];
		a_sr += ns->k3_a_sr[index];
		break;
	default:
		assert(!"Unsupported k_number used as argument.");
		break;
	}
	
	const double a_K_inf = 1.0 / (1.0 + exp(-nrn->s_K * (V - nrn->V_0K))); /* check */
	return (nrn->phi / nrn->tau_K) * (a_K_inf - a_K); /* check */
}

static double neuron_da_sd_wrt_dt(neural_network ns, uint index, enum runge_kutta_4_k_context k_number)
{
	assert("Given index must be a valid number in the range [0, count)."
	       && index >= 0
	       && index < ns->count);

	const struct neuron_profile *nrn = ns->profiles[index];
	double V = ns->V[index];
	double a_K = ns->a_K[index];
	double a_sd = ns->a_sd[index];
	double a_sr = ns->a_sr[index];

	switch (k_number) {
	case RK4_K_1:
		break;
	case RK4_K_2:
		V    += ns->k1_V[index] / 2.0;
		a_K  += ns->k1_a_K[index] / 2.0;
		a_sd += ns->k1_a_sd[index] / 2.0;
		a_sr += ns->k1_a_sr[index] / 2.0;
		break;
	case RK4_K_3:
		V    += ns->k2_V[index] / 2.0;
		a_K  += ns->k2_a_K[index] / 2.0;
		a_sd += ns->k2_a_sd[index] / 2.0;
		a_sr += ns->k2_a_sr[index] / 2.0;
		break;
	case RK4_K_4:
		V    += ns->k3_V[index];
		a_K  += ns->k3_a_K[index];
		a_sd += ns->k3_a_sd[index];
		a_sr += ns->k3_a_sr[index];
		break;
	default:
		assert(!"Unsupported k_number used as argument.");
		break;
	}

	const double a_sd_inf = 1.0 / (1.0 + exp(-nrn->s_sd * (V - nrn->V_0sd)));
	return (nrn->phi / nrn->tau_sd) * (a_sd_inf - a_sd);
}

static double neuron_da_sr_wrt_dt(neural_network ns, uint index, enum runge_kutta_4_k_context k_number)
{
	assert("Given index must be a valid number in the range [0, count)."
	       && index >= 0
	       && index < ns->count);

	const struct neuron_profile *nrn = ns->profiles[index];
	double V = ns->V[index];
	double a_K = ns->a_K[index];
	double a_sd = ns->a_sd[index];
	double a_sr = ns->a_sr[index];

	switch (k_number) {
	case RK4_K_1:
		break;
	case RK4_K_2:
		V    += ns->k1_V[index] / 2.0;
		a_K  += ns->k1_a_K[index] / 2.0;
		a_sd += ns->k1_a_sd[index] / 2.0;
		a_sr += ns->k1_a_sr[index] / 2.0;
		break;
	case RK4_K_3:
		V    += ns->k2_V[index] / 2.0;
		a_K  += ns->k2_a_K[index] / 2.0;
		a_sd += ns->k2_a_sd[index] / 2.0;
		a_sr += ns->k2_a_sr[index] / 2.0;
		break;
	case RK4_K_4:
		V    += ns->k3_V[index];
		a_K  += ns->k3_a_K[index];
		a_sd += ns->k3_a_sd[index];
		a_sr += ns->k3_a_sr[index];
		break;
	default:
		assert(!"Unsupported k_number used as argument.");
		break;
	}
	
	const double I_sd = nrn->rho * nrn->g_sd * a_sd * (V - nrn->V_sd);
	return -(nrn->phi / nrn->tau_sr) * (nrn->v_acc * I_sd + nrn->v_dep * a_sr);
}

neural_network neural_network_create(uint count,
				     void (*neuron_init)(uint i, uint count,
							 double *V, double *a_K, double *a_sd, double *a_sr,
							 struct neuron_profile **np),
				     adj_matrix am)
{
	neural_network result = malloc(sizeof *result);
	
	result->V        = malloc(sizeof (double) * count);
	result->a_K      = malloc(sizeof (double) * count);
	result->a_sd     = malloc(sizeof (double) * count);
	result->a_sr     = malloc(sizeof (double) * count);
	result->k1_V     = malloc(sizeof (double) * count);
	result->k1_a_K   = malloc(sizeof (double) * count);
	result->k1_a_sd  = malloc(sizeof (double) * count);
	result->k1_a_sr  = malloc(sizeof (double) * count);
	result->k2_V     = malloc(sizeof (double) * count);
	result->k2_a_K   = malloc(sizeof (double) * count);
	result->k2_a_sd  = malloc(sizeof (double) * count);
	result->k2_a_sr  = malloc(sizeof (double) * count);
	result->k3_V     = malloc(sizeof (double) * count);
	result->k3_a_K   = malloc(sizeof (double) * count);
	result->k3_a_sd  = malloc(sizeof (double) * count);
	result->k3_a_sr  = malloc(sizeof (double) * count);
	result->k4_V     = malloc(sizeof (double) * count);
	result->k4_a_K   = malloc(sizeof (double) * count);
	result->k4_a_sd  = malloc(sizeof (double) * count);
	result->k4_a_sr  = malloc(sizeof (double) * count);

	result->profiles = malloc((sizeof *(result->profiles)) * count);
	
	result->count = count;
	result->time = 0.0;
	result->adj_matrix = am;
	
	for (uint i = 0; i < count; i++) {
		neuron_init(i, count,
			    &result->V[i], &result->a_K[i], &result->a_sd[i], &result->a_sr[i],
			    &result->profiles[i]);
	}
	
	return result;
}

void neural_network_integrate(neural_network ns, double time_step)
{	
	for (uint i = 0; i < ns->count; i++) {
		ns->k1_V[i]    = time_step * neuron_dV_wrt_dt(ns, i, RK4_K_1);
		ns->k1_a_K[i]  = time_step * neuron_da_K_wrt_dt(ns, i, RK4_K_1);
		ns->k1_a_sd[i] = time_step * neuron_da_sd_wrt_dt(ns, i, RK4_K_1);
		ns->k1_a_sr[i] = time_step * neuron_da_sr_wrt_dt(ns, i, RK4_K_1);
	}

	for (uint i = 0; i < ns->count; i++) {
		ns->k2_V[i]    = time_step * neuron_dV_wrt_dt(ns, i, RK4_K_2);
		ns->k2_a_K[i]  = time_step * neuron_da_K_wrt_dt(ns, i, RK4_K_2);
		ns->k2_a_sd[i] = time_step * neuron_da_sd_wrt_dt(ns, i, RK4_K_2);
		ns->k2_a_sr[i] = time_step * neuron_da_sr_wrt_dt(ns, i, RK4_K_2);
	}

	for (uint i = 0; i < ns->count; i++) {
		ns->k3_V[i]    = time_step * neuron_dV_wrt_dt(ns, i, RK4_K_3);
		ns->k3_a_K[i]  = time_step * neuron_da_K_wrt_dt(ns, i, RK4_K_3);
		ns->k3_a_sd[i] = time_step * neuron_da_sd_wrt_dt(ns, i, RK4_K_3);
		ns->k3_a_sr[i] = time_step * neuron_da_sr_wrt_dt(ns, i, RK4_K_3);
	}

	for (uint i = 0; i < ns->count; i++) {
		ns->k4_V[i]    = time_step * neuron_dV_wrt_dt(ns, i, RK4_K_4);
		ns->k4_a_K[i]  = time_step * neuron_da_K_wrt_dt(ns, i, RK4_K_4);
		ns->k4_a_sd[i] = time_step * neuron_da_sd_wrt_dt(ns, i, RK4_K_4);
		ns->k4_a_sr[i] = time_step * neuron_da_sr_wrt_dt(ns, i, RK4_K_4);
	}
	
	for (uint i = 0; i < ns->count; i++) {
		ns->V[i]    += ns->k1_V[i]/6.0    + ns->k2_V[i]/3.0    + ns->k3_V[i]/3.0    + ns->k4_V[i]/6.0;
		ns->a_K[i]  += ns->k1_a_K[i]/6.0  + ns->k2_a_K[i]/3.0  + ns->k3_a_K[i]/3.0  + ns->k4_a_K[i]/6.0;
		ns->a_sd[i] += ns->k1_a_sd[i]/6.0 + ns->k2_a_sd[i]/3.0 + ns->k3_a_sd[i]/3.0 + ns->k4_a_sd[i]/6.0;
		ns->a_sr[i] += ns->k1_a_sr[i]/6.0 + ns->k2_a_sr[i]/3.0 + ns->k3_a_sr[i]/3.0 + ns->k4_a_sr[i]/6.0;
	}

	ns->time += time_step;
}

double neural_network_get_V(const neural_network ns, uint index)
{
	return ns->V[index];
}

double neural_network_get_a_K(const neural_network ns, uint index)
{
	return ns->a_K[index];
}

double neural_network_get_a_sd(const neural_network ns, uint index)
{
	return ns->a_sd[index];
}

double neural_network_get_a_sr(const neural_network ns, uint index)
{
	return ns->a_sr[index];
}

double neural_network_get_time(const neural_network ns)
{
	return ns->time;
}

uint neural_network_get_count(const neural_network ns)
{
	return ns->count;
}

void neural_network_destroy(neural_network *ns)
{
	free((*ns)->V);
	free((*ns)->a_K);
	free((*ns)->a_sd);
	free((*ns)->a_sr);
	free((*ns)->k1_V);
	free((*ns)->k1_a_K);
	free((*ns)->k1_a_sd);
	free((*ns)->k1_a_sr);
	free((*ns)->k2_V);
	free((*ns)->k2_a_K);
	free((*ns)->k2_a_sd);
	free((*ns)->k2_a_sr);
	free((*ns)->k3_V);
	free((*ns)->k3_a_K);
	free((*ns)->k3_a_sd);
	free((*ns)->k3_a_sr);
	free((*ns)->k4_V);
	free((*ns)->k4_a_K);
	free((*ns)->k4_a_sd);
	free((*ns)->k4_a_sr);
	free((*ns)->profiles);
	free(*ns);
	*ns = NULL;
}
