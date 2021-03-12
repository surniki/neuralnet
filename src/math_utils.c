
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include "headers/math_utils.h"
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
