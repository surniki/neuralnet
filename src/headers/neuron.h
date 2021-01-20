
#ifndef NEURON_H
#define NEURON_H

typedef unsigned int uint;

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
	double v_sr;
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
};

typedef struct neural_network *neural_network;

neural_network neural_network_create(uint width,
				     uint height,
				     void (*coupling_relation)(uint i, uint size, uint *values),
				     void (*neuron_init)(uint i, uint w, uint h, struct neuron *n),
				     const char *data_directory_name);

void neural_network_integrate(neural_network ns, double time_step);

void neural_network_print(neural_network ns);

void neural_network_destroy(neural_network *ns);

#endif
