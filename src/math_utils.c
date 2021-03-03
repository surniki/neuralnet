
#include <assert.h>
#include <math.h>
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
