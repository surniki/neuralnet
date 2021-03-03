
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "headers/test_neuron.h"
#include "../headers/neuron.h"
#include "../headers/adj_matrix.h"
#include "../headers/math_utils.h"
#include "headers/test_utils.h"

static struct neuron_profile tonic_profile = {
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

static struct neuron_profile bursting_profile = {
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

static void neuron_init_callback_double_center(uint i, uint count,
					       double *V, double *a_K, double *a_sd, double *a_sr,
					       struct neuron_profile **np)
{
	uint side_length = sqrt(count);
	uint center = side_length * (side_length / 2) + (side_length / 2);
	uint offset = side_length / 4;

	if (i == center + offset || i == center - offset)
		*np = &tonic_profile;
	else
		*np = &bursting_profile;
	
	*V    = 0.0;
	*a_K  = 0.0;
	*a_sd = 0.0;
	*a_sr = 0.0;
}

static void neuron_init_callback_single_center(uint i, uint count,
					       double *V, double *a_K, double *a_sd, double *a_sr,
					       struct neuron_profile **np)
{
	uint side_length = sqrt(count);
	uint center = side_length * (side_length / 2) + (side_length / 2);

	if (i == center)
		*np   = &tonic_profile;
	else
		*np   = &bursting_profile;

	*V    = 0.0;
	*a_K  = 0.0;
	*a_sd = 0.0;
	*a_sr = 0.0;
}

static void neuron_init_callback_bursting(uint i, uint count,
					  double *V, double *a_K, double *a_sd, double *a_sr,
					  struct neuron_profile **np)
{
	*np   = &bursting_profile;
	*V    = 0.0;
	*a_K  = 0.0;
	*a_sd = 0.0;
	*a_sr = 0.0;
}

static void neuron_init_callback_tonic(uint i, uint count,
				       double *V, double *a_K, double *a_sd, double *a_sr,
				       struct neuron_profile **np)
{
	*np   = &tonic_profile;
	*V    = 0.0;
	*a_K  = 0.0;
	*a_sd = 0.0;
	*a_sr = 0.0;
}

bool test_neural_network_create_destroy(void)
{
	size_t prev_alloc = current_number_of_allocations();
	neural_network ns = neural_network_create(50, &neuron_init_callback_tonic, NULL);
	neural_network_destroy(&ns);
	bool test_1 = ns == NULL;
	bool test_2 = current_number_of_allocations() == prev_alloc;
	
	return test_1 && test_2;
}

const double tolerance = 0.0001;
bool test_neural_network_integrate_single_tonic(void)
{
	bool test = true;

	FILE *data_file = fopen("src/tests/data/single_tonic/single_tonic.dat", "r");
	if (!data_file) {
		return false;
	}

	double time_step = 0.1;
	double final_time = 5000;
	adj_matrix am = adj_matrix_create(1);
	adj_matrix_set_empty(am);
	neural_network ns = neural_network_create(1, &neuron_init_callback_tonic, am);

	double sim_time;	
	while ((sim_time = neural_network_get_time(ns)) < final_time) {
		double n_time = neural_network_get_time(ns);
		double n_V = neural_network_get_V(ns, 0);
		double n_a_K = neural_network_get_a_K(ns, 0);
		double n_a_sd = neural_network_get_a_sd(ns, 0);
		double n_a_sr = neural_network_get_a_sr(ns, 0);

		double c_time, c_V, c_a_K, c_a_sd, c_a_sr;
		if (fscanf(data_file, "%le %le %le %le %le", &c_time, &c_V, &c_a_K, &c_a_sd, &c_a_sr) != 5) {
			return false;
		}

		test = test && math_utils_equal_within_tolerance(n_time, c_time, tolerance);
		test = test && math_utils_equal_within_tolerance(n_V, c_V, tolerance);
		test = test && math_utils_equal_within_tolerance(n_a_K, c_a_K, tolerance);
		test = test && math_utils_equal_within_tolerance(n_a_sd, c_a_sd, tolerance);
		test = test && math_utils_equal_within_tolerance(n_a_sr, c_a_sr, tolerance);

		neural_network_integrate(ns, time_step);
	}

	fclose(data_file);
	neural_network_destroy(&ns);
	adj_matrix_destroy(&am);
	return test;
}

bool test_neural_network_integrate_single_bursting(void)
{
	bool test = true;

	FILE *data_file = fopen("src/tests/data/single_bursting/single_bursting.dat", "r");
	if (!data_file) {
		return false;
	}

	double time_step = 0.1;
	double final_time = 5000;
	adj_matrix am = adj_matrix_create(1);
	adj_matrix_set_empty(am);
	neural_network ns = neural_network_create(1, &neuron_init_callback_bursting, am);

	double sim_time;	
	while ((sim_time = neural_network_get_time(ns)) < final_time) {
		double n_time = neural_network_get_time(ns);
		double n_V = neural_network_get_V(ns, 0);
		double n_a_K = neural_network_get_a_K(ns, 0);
		double n_a_sd = neural_network_get_a_sd(ns, 0);
		double n_a_sr = neural_network_get_a_sr(ns, 0);

		double c_time, c_V, c_a_K, c_a_sd, c_a_sr;
		if (fscanf(data_file, "%le %le %le %le %le", &c_time, &c_V, &c_a_K, &c_a_sd, &c_a_sr) != 5) {
			return false;
		}

		test = test && math_utils_equal_within_tolerance(n_time, c_time, tolerance);
		test = test && math_utils_equal_within_tolerance(n_V, c_V, tolerance);
		test = test && math_utils_equal_within_tolerance(n_a_K, c_a_K, tolerance);
		test = test && math_utils_equal_within_tolerance(n_a_sd, c_a_sd, tolerance);
		test = test && math_utils_equal_within_tolerance(n_a_sr, c_a_sr, tolerance);

		neural_network_integrate(ns, time_step);
	}

	fclose(data_file);
	neural_network_destroy(&ns);
	adj_matrix_destroy(&am);
	return test;
}

bool test_neural_network_integrate_3x3_uncoupled(void)
{
	bool test = true;

	FILE *data_file[] = {
		fopen("src/tests/data/uncoupled_3x3_center_tonic_outside_bursting/0.dat", "r"),
		fopen("src/tests/data/uncoupled_3x3_center_tonic_outside_bursting/1.dat", "r"),
		fopen("src/tests/data/uncoupled_3x3_center_tonic_outside_bursting/2.dat", "r"),
		fopen("src/tests/data/uncoupled_3x3_center_tonic_outside_bursting/3.dat", "r"),
		fopen("src/tests/data/uncoupled_3x3_center_tonic_outside_bursting/4.dat", "r"),
		fopen("src/tests/data/uncoupled_3x3_center_tonic_outside_bursting/5.dat", "r"),
		fopen("src/tests/data/uncoupled_3x3_center_tonic_outside_bursting/6.dat", "r"),
		fopen("src/tests/data/uncoupled_3x3_center_tonic_outside_bursting/7.dat", "r"),
		fopen("src/tests/data/uncoupled_3x3_center_tonic_outside_bursting/8.dat", "r")
	};

	for (int i = 0; i < 9; i++) {
		if (!data_file[i]) {
			for (int j = 0; j < i; j++) {
				fclose(data_file[j]);
			}
			return false;
		}
	}

	double time_step = 0.1;
	double final_time = 5000;
	adj_matrix am = adj_matrix_create(9);
	adj_matrix_set_empty(am);
	neural_network ns = neural_network_create(9, &neuron_init_callback_single_center, am);

	double sim_time;	
	while ((sim_time = neural_network_get_time(ns)) < final_time) {
		double n_time = neural_network_get_time(ns);
		double n_V[9];
		double n_a_K[9];
		double n_a_sd[9];
		double n_a_sr[9];
		for (int i = 0; i < 9; i++) {
			n_V[i] = neural_network_get_V(ns, i);
			n_a_K[i] = neural_network_get_a_K(ns, i);
			n_a_sd[i] = neural_network_get_a_sd(ns, i);
			n_a_sr[i] = neural_network_get_a_sr(ns, i);
		}

		double c_time, c_V[9], c_a_K[9], c_a_sd[9], c_a_sr[9];
		for(int i = 0; i < 9; i++) {
			if (fscanf(data_file[i], "%le %le %le %le %le",
				   &c_time, &c_V[i], &c_a_K[i], &c_a_sd[i], &c_a_sr[i]) != 5) {
				test = false;
				break;
			}
		}

		test = test && math_utils_equal_within_tolerance(n_time, c_time, tolerance);
		for (int i = 0; i < 9; i++) {
			test = test && math_utils_equal_within_tolerance(n_V[i], c_V[i], tolerance);
			test = test && math_utils_equal_within_tolerance(n_a_K[i], c_a_K[i], tolerance);
			test = test && math_utils_equal_within_tolerance(n_a_sd[i], c_a_sd[i], tolerance);
			test = test && math_utils_equal_within_tolerance(n_a_sr[i], c_a_sr[i], tolerance);
		}

		neural_network_integrate(ns, time_step);
	}

	for (int i = 0; i < 9; i++) {
		fclose(data_file[i]);
	}
	
	neural_network_destroy(&ns);
	adj_matrix_destroy(&am);
	
	return test;
}

bool test_neural_network_integrate_3x3_coupled(void)
{
	bool test = true;

	FILE *data_file[] = {
		fopen("src/tests/data/coupled_3x3_center_tonic_outside_bursting/0.dat", "r"),
		fopen("src/tests/data/coupled_3x3_center_tonic_outside_bursting/1.dat", "r"),
		fopen("src/tests/data/coupled_3x3_center_tonic_outside_bursting/2.dat", "r"),
		fopen("src/tests/data/coupled_3x3_center_tonic_outside_bursting/3.dat", "r"),
		fopen("src/tests/data/coupled_3x3_center_tonic_outside_bursting/4.dat", "r"),
		fopen("src/tests/data/coupled_3x3_center_tonic_outside_bursting/5.dat", "r"),
		fopen("src/tests/data/coupled_3x3_center_tonic_outside_bursting/6.dat", "r"),
		fopen("src/tests/data/coupled_3x3_center_tonic_outside_bursting/7.dat", "r"),
		fopen("src/tests/data/coupled_3x3_center_tonic_outside_bursting/8.dat", "r")
	};

	for (int i = 0; i < 9; i++) {
		if (!data_file[i]) {
			for (int j = 0; j < i; j++) {
				fclose(data_file[j]);
			}
			return false;
		}
	}

	double time_step = 0.1;
	double final_time = 5000;
	adj_matrix am = adj_matrix_create(9);
	adj_matrix_set_lattice(am, 3, 3, 0.1);
	neural_network ns = neural_network_create(9, &neuron_init_callback_single_center, am);

	double sim_time;	
	while ((sim_time = neural_network_get_time(ns)) < final_time) {
		double n_time = neural_network_get_time(ns);
		double n_V[9];
		double n_a_K[9];
		double n_a_sd[9];
		double n_a_sr[9];
		for (int i = 0; i < 9; i++) {
			n_V[i] = neural_network_get_V(ns, i);
			n_a_K[i] = neural_network_get_a_K(ns, i);
			n_a_sd[i] = neural_network_get_a_sd(ns, i);
			n_a_sr[i] = neural_network_get_a_sr(ns, i);
		}

		double c_time, c_V[9], c_a_K[9], c_a_sd[9], c_a_sr[9];
		for(int i = 0; i < 9; i++) {
			if (fscanf(data_file[i], "%le %le %le %le %le",
				   &c_time, &c_V[i], &c_a_K[i], &c_a_sd[i], &c_a_sr[i]) != 5) {
				test = false;
				break;
			}
		}

		test = test && math_utils_equal_within_tolerance(n_time, c_time, tolerance);
		for (int i = 0; i < 9; i++) {
			test = test && math_utils_equal_within_tolerance(n_V[i], c_V[i], tolerance);
			test = test && math_utils_equal_within_tolerance(n_a_K[i], c_a_K[i], tolerance);
			test = test && math_utils_equal_within_tolerance(n_a_sd[i], c_a_sd[i], tolerance);
			test = test && math_utils_equal_within_tolerance(n_a_sr[i], c_a_sr[i], tolerance);
		}

		neural_network_integrate(ns, time_step);
	}

	for (int i = 0; i < 9; i++) {
		fclose(data_file[i]);
	}
	
	neural_network_destroy(&ns);
	adj_matrix_destroy(&am);
	
	return test;
}
