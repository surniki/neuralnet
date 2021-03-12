
#ifndef TEST_NEURON_H
#define TEST_NEURON_H

#include <stdbool.h>

bool test_neural_network_create_destroy(void);
bool test_neural_network_integrate_single_tonic(void);
bool test_neural_network_integrate_single_bursting(void);
bool test_neural_network_integrate_3x3_uncoupled(void);
bool test_neural_network_integrate_3x3_coupled(void);
bool test_neural_network_integrate_single_tonic_backwards(void);
bool test_neural_network_integrate_3x3_coupled_backwards(void);
#endif
