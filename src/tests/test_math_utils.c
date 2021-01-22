
#include "headers/test_math_utils.h"
#include "../headers/math_utils.h"
#include "headers/test_utils.h"

bool test_wrap_around(void)
{
	bool test_1 = wrap_around(11, 1, 10) == 1;
	bool test_2 = wrap_around(5, 1, 10) == 5;
	bool test_3 = wrap_around(12, -10, 10) == -9;
	bool test_4 = wrap_around(-10, -3, -1) == -1;

	return test_1 && test_2 && test_3 && test_4;
}
