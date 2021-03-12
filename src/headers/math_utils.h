
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "deftypes.h"
#include <stdbool.h>

int math_utils_wrap_around(int given, int lower, int upper);
bool math_utils_equal_within_tolerance(double v1, double v2, double tolerance);
void math_utils_lattice_indices(uint i, uint width, uint height,
				uint *top, uint *right, uint *bottom, uint *left);
bool math_utils_near_every(double value, float increment, float target_divisor);
double math_utils_random_number(double lowest, double highest);

#endif
