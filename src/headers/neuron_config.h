
#ifndef NEURON_CONFIG_H
#define NEURON_CONFIG_H

struct neuron_profile;

struct neuron_init_entry {
	const char *name;
	const char *desc;
	void (*callback) (uint i, uint count,
			  double *V, double *a_K, double *a_sd, double *a_sr,
			  struct neuron_profile **np);
};

#define neuron_init_entry(command_name, callback_name, description) (struct neuron_init_entry){ .name = command_name, .callback = &callback_name , .desc = description }
#define neuron_init_null_entry  (struct neuron_init_entry){ .callback = (void *)0, .name = (void *)0, .desc = (void *)0 }

struct adj_matrix_init_entry {
	const char *name;
	const char *desc;
	double (*callback)(uint size, uint row, uint col);
};

#define adj_matrix_init_entry(command_name, callback_name, description) (struct adj_matrix_init_entry){ .callback = &callback_name, .name = command_name , .desc = description }
#define adj_matrix_init_null_entry  (struct adj_matrix_init_entry){ .callback = (void *)0, .name = (void *)0, .desc = (void *)0 }

bool is_neuron_init_entry_empty(struct neuron_init_entry e);
bool is_adj_matrix_init_entry_empty(struct adj_matrix_init_entry e);

void neuron_config_coupling_is_random_set(bool value, double lowest, double highest);
void neuron_config_coupling_constant_set(double value);

void neuron_init_callback_double_center(uint i, uint count,
					double *V, double *a_K, double *a_sd, double *a_sr,
					struct neuron_profile **np);

void neuron_init_callback_single_center(uint i, uint count,
					double *V, double *a_K, double *a_sd, double *a_sr,
					struct neuron_profile **np);

void neuron_init_callback_bursting(uint i, uint count,
				   double *V, double *a_K, double *a_sd, double *a_sr,
				   struct neuron_profile **np);

void neuron_init_callback_tonic(uint i, uint count,
				double *V, double *a_K, double *a_sd, double *a_sr,
				struct neuron_profile **np);

double adj_matrix_init_callback_empty(uint size, uint row, uint col);
double adj_matrix_init_callback_lattice(uint size, uint row, uint col);
double adj_matrix_init_callback_complete(uint size, uint row, uint col);

#endif
