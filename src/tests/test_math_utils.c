
#include "headers/test_math_utils.h"
#include "../headers/math_utils.h"
#include "../headers/temp_memory.h"
#include "headers/test_utils.h"

bool test_math_utils_wrap_around(void)
{
	bool test_1 = math_utils_wrap_around(11, 1, 10) == 1;
	bool test_2 = math_utils_wrap_around(5, 1, 10) == 5;
	bool test_3 = math_utils_wrap_around(12, -10, 10) == -9;
	bool test_4 = math_utils_wrap_around(-10, -3, -1) == -1;

	return test_1 && test_2 && test_3 && test_4;
}

bool test_math_utils_equal_within_tolerance(void)
{
	bool test_1 = math_utils_equal_within_tolerance(42.0, 42.0, 0.0);
	bool test_2 = math_utils_equal_within_tolerance(42.0, 42.0, 0.0001);
	bool test_3 = !math_utils_equal_within_tolerance(42.0, 42.01, 0.0001);
	bool test_4 = math_utils_equal_within_tolerance(42.0, 42.00001, 0.0001);

	return test_1 && test_2 && test_3 && test_4;
}

bool test_math_utils_lattice_indices(void)
{
	uint top, right, bottom, left;
	math_utils_lattice_indices(0, 3, 3, &top, &right, &bottom, &left);
	bool test_1 = top == 6 && right == 1 && bottom == 3 && left == 2;
	math_utils_lattice_indices(1, 3, 3, &top, &right, &bottom, &left);
	bool test_2 = top == 7 && right == 2 && bottom == 4 && left == 0;
	math_utils_lattice_indices(2, 3, 3, &top, &right, &bottom, &left);
	bool test_3 = top == 8 && right == 0 && bottom == 5 && left == 1;
	math_utils_lattice_indices(3, 3, 3, &top, &right, &bottom, &left);
	bool test_4 = top == 0 && right == 4 && bottom == 6 && left == 5;
	math_utils_lattice_indices(4, 3, 3, &top, &right, &bottom, &left);
	bool test_5 = top == 1 && right == 5 && bottom == 7 && left == 3;
	math_utils_lattice_indices(5, 3, 3, &top, &right, &bottom, &left);
	bool test_6 = top == 2 && right == 3 && bottom == 8 && left == 4;
	math_utils_lattice_indices(6, 3, 3, &top, &right, &bottom, &left);
	bool test_7 = top == 3 && right == 7 && bottom == 0 && left == 8;
	math_utils_lattice_indices(7, 3, 3, &top, &right, &bottom, &left);
	bool test_8 = top == 4 && right == 8 && bottom == 1 && left == 6;
	math_utils_lattice_indices(8, 3, 3, &top, &right, &bottom, &left);
	bool test_9 = top == 5 && right == 6 && bottom == 2 && left == 7;

	return test_1 && test_2 && test_3 && test_4 && test_5 && test_6
		&& test_7 && test_8 && test_9;
}

double gravity = -9.81;
static void *parameter_callback(dynamical_system ds, uint index)
{
	return &gravity;
}
static double coupling_callback(dynamical_system ds, uint first_index, uint second_index)
{
	return 0.0;
}
static void initial_values_callback(uint index, uint size, double *system_values)
{
	if (index == 0) {
		system_values[0] = 0.0;
		system_values[1] = 100.0;
	}
	else {
		system_values[0] = 2.0;
		system_values[1] = 50.0;
	}	
}
static double dv_wrt_dt(dynamical_system ds, uint index)
{
	double *g = dynamical_system_get_parameters(ds, index);
	return *g;
}
static double dx_wrt_dt(dynamical_system ds, uint index)
{
	return dynamical_system_get_value(ds, index, 0);
}
static double analytical_solution_x(double t, double initial_x, double initial_v)
{
	return 0.5*gravity*t*t + initial_v*t + initial_x;
}

bool test_math_utils_rk4_integrate(void)
{
	double step = 0.005;
	const double tol = 1;
	double (*derivatives[]) (dynamical_system, uint) = { &dv_wrt_dt, &dx_wrt_dt };

	const double first_initial_v = 0.0;
	const double first_initial_x = 100.0;
	const double second_initial_v = 2.0;
	const double second_initial_x = 50.0;
	
	dynamical_system free_fall_objects =
		dynamical_system_create(2, 2,
					parameter_callback,
					coupling_callback,
					initial_values_callback,
					derivatives);     
	
	uint previous_allocations = current_number_of_allocations();

	bool test_1 = true;
	for (uint i = 0; i < 100; i++) {
		double time = dynamical_system_get_time(free_fall_objects);
		double first_a_x = analytical_solution_x(time, first_initial_x, first_initial_v);
		double second_a_x = analytical_solution_x(time, second_initial_x, second_initial_v);
		double first_d_x = dynamical_system_get_value(free_fall_objects, 0, 1);
		double second_d_x = dynamical_system_get_value(free_fall_objects, 1, 1);
		bool first_test = math_utils_equal_within_tolerance(first_a_x, first_d_x, tol);
		bool second_test = math_utils_equal_within_tolerance(second_a_x, second_d_x, tol);
		test_1 = test_1 && first_test && second_test;
		math_utils_rk4_integrate(free_fall_objects, step);
	}

	temp_free();

	dynamical_system_destroy(&free_fall_objects);
	
	bool test_2 = current_number_of_allocations() == previous_allocations;

	return test_1 && test_2;
}
