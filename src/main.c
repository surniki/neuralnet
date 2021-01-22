
#include <stdio.h>
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
	.V_sr = -90.0,
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
	.V_sr = -90.0,
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

int main(void)
{
	puts("neuralnet running");
	return 0;
}
