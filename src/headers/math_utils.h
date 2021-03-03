
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdbool.h>

int math_utils_wrap_around(int given, int lower, int upper);
bool math_utils_equal_within_tolerance(double v1, double v2, double tolerance);

#endif
