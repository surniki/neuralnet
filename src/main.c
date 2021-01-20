
#include "headers/neuron.h"

const struct neuron_profile neuron_profile_tonic = {
	.I_inj = 0.0,
	.C = 1.0,
	.g_leak = 0.1,
	.V_leak = -60.0,
	.rho = 0.607,
	.g_Na = 1.5,
	.V_Na = 50.0,
	.g_K = 2.0,
	.V_K = -90,
	.g_sd = 0.25,
	.V_sd = 50,
	.g_sr = 0.25,
	.v_sr = -90.0,
	.s_Na = 0.25,
	.V_0Na = -25.0,
	.phi = 0.124,
	.tau_K = 2.0,
	.tau_sd = 10.0,
	.tau_sr = 20.0,
	.v_acc = 0.17,
	.v_dep = 0.012,
	.s_K = 0.25,
	.V_0K = -25.0,
	.s_sd = 0.09,
	.V_0sd = -40.0
};

const struct neuron_profile neuron_profile_bursting = {
	.I_inj = 0.0,
	.C = 1.0,
	.g_leak = 0.1,
	.V_leak = -60.0,
	.rho = 0.607,
	.g_Na = 1.5,
	.V_Na = 50.0,
	.g_K = 2.0,
	.V_K = -90,
	.g_sd = 0.25,
	.V_sd = 50,
	.g_sr = 0.45,
	.v_sr = -90.0,
	.s_Na = 0.25,
	.V_0Na = -25.0,
	.phi = 0.124,
	.tau_K = 2.0,
	.tau_sd = 10.0,
	.tau_sr = 20.0,
	.v_acc = 0.17,
	.v_dep = 0.012,
	.s_K = 0.25,
	.V_0K = -25.0,
	.s_sd = 0.09,
	.V_0sd = -40.0
};

void coupling_relation_lattice(uint i, uint size, uint *values)
{
	
}

void neuron_init_swp(uint i, uint w, uint h, struct neuron *n)
{

}

int main(void)
{
	neural_network ns = neural_network_create(10, 10,
						  &coupling_relation_lattice,
						  &neuron_init_swp,
						  "nn_output");

	for (int i = 0; i < 1000; i++) {
		neural_network_integrate(ns, 0.01);
		neural_network_print(ns);
	}
	
	neural_network_destroy(&ns);
	return 0;
}
