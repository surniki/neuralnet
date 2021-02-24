
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "headers/deftypes.h"
#include "headers/timer.h"
#include "headers/file_table.h"
#include "headers/adj_matrix.h"
#include "headers/neuron.h"

struct neuron_profile tonic_profile = {
	.I_inj = 1.0,     /* checked, should have frequency ~8Hz */
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

struct neuron_profile bursting_profile = {
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

static void neuron_init_callback(uint i, uint count,
				 double *V, double *a_K, double *a_sd, double *a_sr,
				 struct neuron_profile **np)
{
	if (i == 0)
		*np = &bursting_profile;
	else
		*np = &tonic_profile;
	
	*V    = 0.0;
	*a_K  = 0.0;
	*a_sd = 0.0;
	*a_sr = 0.0;
}

static double element_setter_callback(uint size, uint row, uint col)
{
	if (row == col - 1 || row - 1 == col)
		return 0.0;
	else
		return 0.0;
}

int main(void)
{
	const double g_c = 0.1;
	const uint lattice_width = 4;
	const uint lattice_height = 4;
	const uint neuron_count = lattice_width * lattice_height;
	const double time_step = 0.01;
	const double final_time = 10000;
	const double progress_print_interval = 1.0;

	file_table fs = file_table_create("output", neuron_count, 1, "voltage_matrix.dat");
	if (!fs) {
		puts("Fatal error: Could not create the file table.");
		return 0;
	}
	
	adj_matrix am = adj_matrix_create(neuron_count);
	adj_matrix_set_lattice(am, lattice_width, lattice_height, g_c);
	neural_network ns = neural_network_create(neuron_count, &neuron_init_callback, am);

	timer timer = timer_begin();

	double sim_time;
	while ((sim_time = neural_network_get_time(ns)) < final_time) {
		timer_print(timer, progress_print_interval,
			    "Progress: %3d%%, Time elapsed: %9.2fs\n",
			    (int)(100 * sim_time / final_time),
			    timer_total_get(timer));

		for (uint i = 0; i < neural_network_get_count(ns); i++) {	
			file_table_index_print(fs, i, "%.10e %.10e %.10e %.10e %.10e\n",
					       neural_network_get_time(ns),
					       neural_network_get_V(ns, i),
					       neural_network_get_a_K(ns, i),
					       neural_network_get_a_sd(ns, i),
					       neural_network_get_a_sr(ns, i));
		}

		file_table_special_print(fs, "voltage_matrix.dat", "# time = %f ms\n", sim_time);
		for (uint row = 0; row < lattice_height; row++) {
			for (uint col = 0; col < lattice_width; col++) {
				file_table_special_print(fs, "voltage_matrix.dat", "%.10e ", neural_network_get_V(ns, row * lattice_width + col));
			}
			file_table_special_print(fs, "voltage_matrix.dat", "\n");
		}
		file_table_special_print(fs, "voltage_matrix.dat", "\n");
			
		neural_network_integrate(ns, time_step);
	}

	timer_end(&timer, "Total elapsed time: %.2f\n", timer_total_get(timer));
	
	adj_matrix_destroy(&am);
	neural_network_destroy(&ns);
	file_table_destroy(&fs);
	
	return 0;
}
