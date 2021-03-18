
#include "headers/test_temp_memory.h"
#include "../headers/temp_memory.h"
#include "headers/test_utils.h"

bool test_temp_malloc()
{
	int *test = temp_malloc(sizeof (int) * 5);
	bool test_1 = true;
	for (int i = 0; i < 5; i++) {
		test_1 = test_1 && test[i] == 0;
	}

	temp_release();
	test = temp_malloc(sizeof (int) * 10);
	test[4] = 1;
	test[2] = 1;
	bool test_2 = test[0] == 0 && test[1] == 0 && test[2] == 1 &&
		test[3] == 0 && test[4] == 1 && test[5] == 0 &&
		test[6] == 0 && test[7] == 0 && test[8] == 0 && test[9] == 0;

	temp_free();

	return test_1 && test_2;
}

bool test_temp_free()
{
	size_t previous_allocations = current_number_of_allocations();
	void *data = temp_malloc(64);
	temp_free();

	bool test_1 = previous_allocations - current_number_of_allocations() == 0;
	return test_1;
}
