
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include "headers/math_utils.h"
#include "headers/dynamical_system.h"
#include "headers/temp_memory.h"

#include "tests/headers/test_utils.h"

int math_utils_wrap_around(int given, int lower, int upper)
{
	assert("The interval given must be properly defined." && lower < upper);
	const int width = upper - lower + 1;
	int result = given;

	/* An intentionally simple implementation; will fix if a problem later. */
	if (result < lower)
		while ((result += width) < lower);
	else if (result > upper)
		while ((result -= width) > upper);

	return result;
}

bool math_utils_equal_within_tolerance(double v1, double v2, double tolerance)
{
	return v1 == v2 || fabs(v1 - v2) <= tolerance;
}

void math_utils_lattice_indices(uint i, uint width, uint height,
				uint *top, uint *right, uint *bottom, uint *left)
{
	*top = math_utils_wrap_around(i - width, 0, width * height - 1);
	*right = math_utils_wrap_around(i + 1, width * (i / width), (1 + (i / width)) * width - 1);
	*bottom = math_utils_wrap_around(i + width, 0, width * height - 1);
	*left = math_utils_wrap_around(i - 1, width * (i / width), (1 + (i / width)) * width - 1);
}

void math_utils_all_neighbors_indices(uint i, uint width, uint height,
				      uint *top, uint *right, uint *bottom, uint *left,
				      uint *top_left, uint *top_right, uint *bottom_right, uint *bottom_left)
{
	*top = math_utils_wrap_around(i - width, 0, width * height - 1);
	*right = math_utils_wrap_around(i + 1, width * (i / width), (1 + (i / width)) * width - 1);
	*bottom = math_utils_wrap_around(i + width, 0, width * height - 1);
	*left = math_utils_wrap_around(i - 1, width * (i / width), (1 + (i / width)) * width - 1);

	*top_left = math_utils_wrap_around(*top - 1, (*top / width) * width, (*top / width) * width + (width - 1));
	*top_right = math_utils_wrap_around(*top + 1, (*top / width) * width, (*top / width) * width + (width - 1));
	*bottom_left = math_utils_wrap_around(*bottom - 1, (*bottom / width) * width, (*bottom / width) * width + (width - 1));
	*bottom_right = math_utils_wrap_around(*bottom + 1, (*bottom / width) * width, (*bottom / width) * width + (width - 1));
}

bool math_utils_near_every(double value, float increment, float target_divisor)
{
	const double tolerance = increment / 2;
	const double mod_result = fmod(value, target_divisor);
	return tolerance > mod_result || (target_divisor - tolerance) < mod_result;
}

double math_utils_lerp(double input,
		       double low_input, double high_input, double low_output, double high_output)
{
	return low_output + (input - low_input) *
		(high_output - low_output) / (high_input - low_input);
}

double math_utils_random_number(double lowest, double highest)
{
	static bool is_seeded = false;
	if (!is_seeded) {
		srand(time(NULL));
		is_seeded = true;
	}

	double normalized_random_value = rand() / (double) RAND_MAX;
	return math_utils_lerp(normalized_random_value, 0.0, 1.0, lowest, highest);
}

/* expects number_of_derivatives function pointers, double pointers */
bool math_utils_rk4_integrate(dynamical_system ds, double step)
{
	uint system_size = dynamical_system_get_system_size(ds);
	uint element_size = dynamical_system_get_element_size(ds);
	double *k_memory = temp_malloc((sizeof *k_memory) * system_size * element_size * 4);

	double (**derivatives)(dynamical_system, uint) = dynamical_system_get_derivatives(ds);

#define k(i) (k_memory[(i) * system_size * element_size + system * element_size + element])

	/* first step: inputs x, y */
	for (uint system = 0; system < system_size; system++) {
		for (uint element = 0; element < element_size; element++) {
			k(0) = step * derivatives[element](ds, system);
		}
	}
	/* second step: input x + step / 2, y + k1 / 2 */
	dynamical_system_increment_time(ds, step / 2.0);
	for (uint system = 0; system < system_size; system++) {
		for (uint element = 0; element < element_size; element++) {
			dynamical_system_increment_value(ds, system, element, k(0) / 2.0);
		}
		for (uint element = 0; element < element_size; element++) {
			k(1) = step * derivatives[element](ds, system);
		}
	}
	/* third step: input x + step / 2, y + k2 / 2 */
	for (uint system = 0; system < system_size; system++) {
		for (uint element = 0; element < element_size; element++) {
			dynamical_system_increment_value(ds, system, element, (k(1) - k(0)) / 2.0);
		}
		for (uint element = 0; element < element_size; element++) {
			k(2) = step * derivatives[element](ds, system);
		}
	}
	/* fourth step: input x + step, y + k3 */
	dynamical_system_increment_time(ds, step / 2.0);
	for (uint system = 0; system < system_size; system++) {
		for (uint element = 0; element < element_size; element++) {
			dynamical_system_increment_value(ds, system, element, k(2) - (k(1) / 2.0));
		}
		for (uint element = 0; element < element_size; element++) {
			k(3) = step * derivatives[element](ds, system);
		}
	}
		/* set the new values */
	for (uint system = 0; system < system_size; system++) {
		for (uint element = 0; element < element_size; element++) {
			dynamical_system_increment_value(ds, system, element, -k(2));
			dynamical_system_increment_value(ds, system, element,
							 k(0) / 6.0 +
							 k(1) / 3.0 +
							 k(2) / 3.0 +
							 k(3) / 6.0);
		}
	}
		
	temp_release();
#undef k
}
