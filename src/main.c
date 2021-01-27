
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "headers/deftypes.h"
#include "headers/timer.h"
#include "headers/file_array.h"
#include "headers/adj_matrix.h"
#include "headers/neuron.h"

struct neuron_profile tonic_profile = {
	.I_inj = 0.0,
	.C = 1.0,
	.g_leak = 0.1,
	.V_leak = -60.0,
	.rho = 0.675,
	.g_Na = 1.5,
	.V_Na = 50.0,
	.g_K = 2.0,
	.V_K = -90.0,
	.g_sd = 0.25,
	.V_sd = 50.0,
	.g_sr = 0.25,
	.V_sr = -90.0,
	.s_Na = 0.25,
	.V_0Na = -25.0,
	.phi = 0.192,
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

struct neuron_profile bursting_profile = {
	.I_inj = -20.0,
	.C = 1.0,
	.g_leak = 0.1,
	.V_leak = -60.0,
	.rho = 0.675,
	.g_Na = 1.5,
	.V_Na = 50.0,
	.g_K = 2.0,
	.V_K = -90.0,
	.g_sd = 0.25,
	.V_sd = 50.0,
	.g_sr = 0.25,
	.V_sr = -90.0,
	.s_Na = 0.25,
	.V_0Na = -25.0,
	.phi = 0.192,
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

void neuron_init_callback(uint i, uint count,
			  double *V, double *a_K, double *a_sd, double *a_sr,
			  struct neuron_profile **np)
{
	uint side_len = round(sqrt(count));
	uint row = round(side_len / 2.0);
	uint lcol = round(side_len * (1.0 / 4.0));
	uint rcol = round(side_len * (3.0 / 4.0));

	if (i == row * side_len + lcol || i == row * side_len + rcol) {
		*np = &bursting_profile;
	}
	else {
		*np = &tonic_profile;
	}

	*V    = 0.0;
	*a_K  = 0.5;
	*a_sd = 0.5;
	*a_sr = 0.5;
}

int main(void)
{
	const uint lattice_width = 10;
	const uint lattice_height = 10;
	const uint neuron_count = lattice_width * lattice_height;
	const double time_step = 0.01;
	const double final_time = 1000;
	const double progress_print_interval = 1.0;
	
	file_array fs = file_array_create(neuron_count + 1, "output");
	if (!fs) {
		puts("Fatal error: Could not create the file array.");
		return 0;
	}

	adj_matrix am = adj_matrix_create(neuron_count);
	adj_matrix_set_lattice(am, lattice_width, lattice_height, 0.2);	
	neural_network ns = neural_network_create(neuron_count, &neuron_init_callback, am);

	timer timer = timer_begin();

	double sim_time;
	while ((sim_time = neural_network_get_time(ns)) < final_time) {
		timer_print(timer, progress_print_interval, "Progress: %3d%%, Time elapsed: %9.2fs\n",
			    (int)(100 * sim_time / final_time),
			    timer_total_get(timer));

		for (uint i = 0; i < neural_network_get_count(ns); i++) {	
			file_array_print(fs, i, "%.10e %.10e %.10e %.10e %.10e\n",
					 neural_network_get_time(ns),
					 neural_network_get_V(ns, i),
					 neural_network_get_a_K(ns, i),
					 neural_network_get_a_sd(ns, i),
					 neural_network_get_a_sr(ns, i));
		}
		for (uint row = 0; row < lattice_height; row++) {
			for (uint col = 0; col < lattice_width; col++) {
				file_array_print(fs, file_array_get_length(fs) - 1, "%.10e ",
						 neural_network_get_V(ns, row * lattice_width + col));
			}
			file_array_print(fs, file_array_get_length(fs) - 1, "\n");
		}
		file_array_print(fs, file_array_get_length(fs) - 1, "\n");

		neural_network_integrate(ns, time_step);
	}

	timer_end(&timer, "Total elapsed time: %.2f\n", timer_total_get(timer));
	
	adj_matrix_destroy(&am);
	neural_network_destroy(&ns);
	file_array_destroy(&fs);
	
	return 0;
}
