
#ifndef NEURON_H
#define NEURON_H

#include "deftypes.h"
#include "adj_matrix.h"

struct neuron_profile {
	double I_inj;
	double C;
	double g_leak;
	double V_leak;
	double rho;
	double g_Na;
	double V_Na;
	double g_K;
	double V_K;
	double g_sd;
	double V_sd;
	double g_sr;
	double V_sr;
	double s_Na;
	double V_0Na;
	double phi;
	double tau_K;
	double tau_sd;
	double tau_sr;
	double v_acc;
	double v_dep;
	double s_K;
	double V_0K;
	double s_sd;
	double V_0sd;
};

struct neuron {
	const struct neuron_profile *profile;
	double V, a_K, a_sd, a_sr;
	double prev_V, prev_a_K, prev_a_sd, prev_a_sr;
};

typedef struct neural_network *neural_network;

neural_network neural_network_create(uint width,
				     uint height,
				     void (*neuron_init)(uint i, uint w, uint h, struct neuron *n),
				     adj_matrix (*adj_matrix_init)(uint node_count));

void neural_network_integrate(neural_network ns, double time_step);

void neural_network_print(neural_network ns);

void neural_network_destroy(neural_network *ns);

#endif
