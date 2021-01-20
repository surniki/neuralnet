
#include <stdlib.h>
#include "headers/neuron.h"

struct neural_network {
	uint width, height;
	uint *coupling_relation_buffer;
	struct neuron neurons[];
};

neural_network neural_network_create(uint width,
				     uint height,
				     void (*coupling_relation)(uint i, uint size, uint *values),
				     void (*neuron_init)(uint i, uint w, uint h, struct neuron *n),
				     const char *data_directory_name)
{
	return NULL;
}

void neural_network_integrate(neural_network ns, double time_step)
{

}

void neural_network_print(neural_network ns)
{

}

void neural_network_destroy(neural_network *ns)
{

}
