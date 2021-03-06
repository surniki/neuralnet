
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "deftypes.h"
#include <stdbool.h>
#include "dynamical_system.h"

int math_utils_wrap_around(int given, int lower, int upper);
bool math_utils_equal_within_tolerance(double v1, double v2, double tolerance);
void math_utils_lattice_indices(uint i, uint width, uint height,
				uint *top, uint *right, uint *bottom, uint *left);
void math_utils_all_neighbors_indices(uint i, uint width, uint height,
				      uint *top, uint *right, uint *bottom, uint *left,
				      uint *top_left, uint *top_right, uint *bottom_right, uint *bottom_left);
bool math_utils_near_every(double value, float increment, float target_divisor);
double math_utils_random_number(double lowest, double highest);
double math_utils_lerp(double input,
		       double low_input, double high_input, double low_output, double high_output);
bool math_utils_rk4_integrate(dynamical_system ds, double step);

#endif
