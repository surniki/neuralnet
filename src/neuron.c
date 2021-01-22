
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "headers/deftypes.h"
#include "headers/neuron.h"
#include "headers/file_array.h"
#include "headers/adj_matrix.h"

#include "tests/headers/test_utils.h"

struct neural_network {
	uint count;
	adj_matrix adj_matrix;
	file_array files;
	struct neuron neurons[];
};

static double neuron_dV_wrt_dt(const struct neuron_profile *nrn,
			       double V, double a_K, double a_sd, double a_sr,
			       double I_coupling)
{

	const double I_leak = nrn->g_leak * (V - nrn->V_leak);
	const double a_Na   = 1.0 / (1.0 + exp(-nrn->s_Na * (V - nrn->V_0Na)));

	const double I_Na   = nrn->rho * nrn->g_Na * a_Na * (V - nrn->V_Na);
	const double I_K    = nrn->rho * nrn->g_K  * a_K  * (V - nrn->V_K);
	const double I_sd   = nrn->rho * nrn->g_sd * a_sd * (V - nrn->V_sd);
	const double I_sr   = nrn->rho * nrn->g_sr * a_sr * (V - nrn->V_sr);

	return -(I_leak + I_Na + I_K + I_sd + I_sr + nrn->I_inj + I_coupling) / nrn->C;
}

static double neuron_da_K_wrt_dt(const struct neuron_profile *nrn,
				 double V, double a_K)
{
	const double a_K_inf = 1.0 / (1.0 + exp(-nrn->s_K * (V - nrn->V_0K)));
	return (nrn->phi / nrn->tau_K) * (a_K_inf - a_K);
}

static double neuron_da_sd_wrt_dt(const struct neuron_profile *nrn,
				  double V, double a_sd)
{
	const double a_sd_inf = 1.0 / (1.0 + exp(-nrn->s_sd * (V - nrn->V_0K)));
	return (nrn->phi / nrn->tau_sd) * (a_sd_inf - a_sd);
}

static double neuron_da_sr_wrt_dt(const struct neuron_profile *nrn,
				  double V, double a_sd, double a_sr)
{
	const double I_sd = nrn->rho * nrn->g_sd * a_sd * (V - nrn->V_sd);
	return -(nrn->phi / nrn->tau_sr) * (nrn->v_acc * I_sd + nrn->v_dep * a_sr);
}

neural_network neural_network_create(uint width,
				     uint height,
				     void (*neuron_init)(uint i, uint w, uint h, struct neuron *n),
				     adj_matrix (*adj_matrix_init)(uint node_count))
{
	size_t size = sizeof (struct neural_network) + sizeof (struct neuron) * width * height;
	neural_network result = malloc(size);
	memset(result, 0, size);

	result->count = width * height;
	result->adj_matrix = adj_matrix_init(width * height);
	result->files = file_array_create(width * height, "output");
	for (uint i = 0; i < width * height; i++) {
		neuron_init(i, width, height, result->neurons + i);
	}
	
	return result;
}

void neural_network_integrate(neural_network ns, double time_step)
{	
	for (uint i = 0; i < ns->count; i++) {
		struct neuron *n = ns->neurons + i;
		n->prev_V = n->V;
		n->prev_a_K = n->a_K;
		n->prev_a_sd = n->a_sd;
		n->prev_a_sr = n->a_sr;
	}

	for (uint i = 0; i < ns->count; i++) {
		#define I_V     0
		#define I_A_K   1
		#define I_A_SD  2
		#define I_A_SR  3


		struct neuron *n = ns->neurons + i;
		const double V = n->prev_V;
		const double a_K = n->prev_a_K;
		const double a_sd = n->prev_a_sd;
		const double a_sr = n->prev_a_sr;

		/* TODO: I_coupling needs to reflect the increments made in k1 .. k4 */
		uint c_index;
		double g_c;
		double I_coupling = 0;

		#if 0
		while (g_c = adj_matrix_poll_for_indices(ns->adj_matrix, i, &c_index)) {
			struct neuron *coupled_n = ns->neurons + c_index;
			const double coupled_V = coupled_n->prev_V;
			I_coupling += g_c * (V - coupled_V);
		}
		#endif
		
		double k1[] = {0.0, 0.0, 0.0, 0.0};
		double k2[] = {0.0, 0.0, 0.0, 0.0};
		double k3[] = {0.0, 0.0, 0.0, 0.0};
		double k4[] = {0.0, 0.0, 0.0, 0.0};

		k1[I_V]    = time_step * neuron_dV_wrt_dt(n->profile, V, a_K, a_sd, a_sr, I_coupling);
		k1[I_A_K]  = time_step * neuron_da_K_wrt_dt(n->profile, V, a_K);
		k1[I_A_SD] = time_step * neuron_da_sd_wrt_dt(n->profile, V, a_sd);
		k1[I_A_SR] = time_step * neuron_da_sr_wrt_dt(n->profile, V, a_sd, a_sr);

		k2[I_V]    = time_step * neuron_dV_wrt_dt(n->profile,
							  V + k1[I_V] / 2.0,
							  a_K + k1[I_A_K] / 2.0,
							  a_sd + k1[I_A_SD] / 2.0,
							  a_sr + k1[I_A_SR] / 2.0,
							  I_coupling);
		k2[I_A_K]  = time_step * neuron_da_K_wrt_dt(n->profile,
							    k1[I_V] / 2.0,
							    k1[I_A_K] / 2.0);
		k2[I_A_SD] = time_step * neuron_da_sd_wrt_dt(n->profile,
							     k1[I_V] / 2.0,
							     k1[I_A_SD] / 2.0);
		k2[I_A_SR] = time_step * neuron_da_sr_wrt_dt(n->profile,
							     k1[I_V] / 2.0,
							     k1[I_A_SD] / 2.0,
							     k1[I_A_SR] / 2.0);

		k3[I_V]    = time_step * neuron_dV_wrt_dt(n->profile,
							  k2[I_V] / 2.0,
							  k2[I_A_K] / 2.0,
							  k2[I_A_SD] / 2.0,
							  k2[I_A_SR] / 2.0,
							  I_coupling);
		k3[I_A_K]  = time_step * neuron_da_K_wrt_dt(n->profile,
							    k2[I_V] / 2.0,
							    k2[I_A_K] / 2.0);
		k3[I_A_SD] = time_step * neuron_da_sd_wrt_dt(n->profile,
							     k2[I_V] / 2.0,
							     k2[I_A_SD] / 2.0);
		k3[I_A_SR] = time_step * neuron_da_sr_wrt_dt(n->profile,
							     k2[I_V] / 2.0,
							     k2[I_A_SD] / 2.0,
							     k2[I_A_SR] / 2.0);

		k4[I_V]    = time_step * neuron_dV_wrt_dt(n->profile,
							  k3[I_V],
							  k3[I_A_K],
							  k3[I_A_SD],
							  k3[I_A_SR],
							  I_coupling);
		k4[I_A_K]  = time_step * neuron_da_K_wrt_dt(n->profile,
							    k3[I_V],
							    k3[I_A_K]);
		k4[I_A_SD] = time_step * neuron_da_sd_wrt_dt(n->profile,
							     k3[I_V],
							     k3[I_A_SD]);
		k4[I_A_SR] = time_step * neuron_da_sr_wrt_dt(n->profile,
							     k3[I_V],
							     k3[I_A_SD],
							     k3[I_A_SR]);

		n->V = V + k1[I_V] / 6.0 + k2[I_V] / 3.0 + k3[I_V] / 3.0 + k4[I_V] / 6.0;
		n->a_K = a_K + k1[I_A_K] / 6.0 + k2[I_A_K] / 3.0 + k3[I_A_K] / 3.0 + k4[I_A_K] / 6.0;
		n->a_sd = V + k1[I_A_SD] / 6.0 + k2[I_A_SD] / 3.0 + k3[I_A_SD] / 3.0 + k4[I_A_SD] / 6.0;
		n->a_sr = V + k1[I_A_SR] / 6.0 + k2[I_A_SR] / 3.0 + k3[I_A_SR] / 3.0 + k4[I_A_SR] / 6.0;
		#undef I_V
		#undef I_A_K
		#undef I_A_SD
		#undef I_A_SR
	}
}

void neural_network_print(neural_network ns)
{
	for (uint i = 0; i < ns->count; i++) {
		struct neuron *n = ns->neurons + i;
		file_array_print(ns->files, i, "%4.3e %4.3e %4.3e %4.3e\n", n->V, n->a_K, n->a_sd, n->a_sr);
	}
}

void neural_network_destroy(neural_network *ns)
{
	adj_matrix_destroy(&(*ns)->adj_matrix);
	file_array_destroy(&(*ns)->files);
	free(*ns);
	*ns = NULL;
}
