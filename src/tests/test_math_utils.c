
#include "headers/test_math_utils.h"
#include "../headers/math_utils.h"
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
