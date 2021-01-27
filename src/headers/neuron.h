
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

typedef struct neural_network *neural_network;

enum runge_kutta_4_k_context {
	RK4_K_1,
	RK4_K_2,
	RK4_K_3,
	RK4_K_4
};

neural_network neural_network_create(uint count, 
				     void (*neuron_init)(uint i, uint count,
							 double *V, double *a_K, double *a_sd, double *a_sr,
							 struct neuron_profile **np),
				     adj_matrix am);

void neural_network_integrate(neural_network ns, double time_step);

double neural_network_get_V(const neural_network ns, uint index);
double neural_network_get_a_K(const neural_network ns, uint index);
double neural_network_get_a_sd(const neural_network ns, uint index);
double neural_network_get_a_sr(const neural_network ns, uint index);
double neural_network_get_time(const neural_network ns);
uint neural_network_get_count(const neural_network ns);

void neural_network_destroy(neural_network *ns);

#endif
