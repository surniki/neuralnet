
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "headers/deftypes.h"
#include "headers/timer.h"
#include "headers/file_table.h"
#include "headers/adj_matrix.h"
#include "headers/neuron.h"
#include "headers/math_utils.h"
#include "headers/neuron_config.h"

#define version_statement "neuralnet v0.2: March 2021, Scott Urnikis"
#define hrule \
	"////////////////////////////////////////" \
	"////////////////////////////////////////"
#define help_desc "Print this document."
#define version_desc "Print the version number."
#define visualize_desc "Render the simulation graphically."
#define output_data_desc "Output the simulation data to data files."
#define random_coupling_desc \
	"Takes two additional arguments x, y, where x and y must be within the\n" \
	"interval [0.0, 1.0]. Randomly chooses a number within the given range\n" \
	"to be the edge value representing the coupling constants for the neural\n" \
	"network."
#define coupling_constant_desc \
	"Takes a single additional argument x, where x must be within the\n" \
	"interval [0.0, 1.0]. The coupling constant for all coupled neurons\n" \
	"will be set to x."
#define neuron_count_desc \
	"Takes a single additional argument x, where x must be a positive\n" \
	"integer. The number of neurons to simulate will be set to x."
#define time_step_desc \
	"Takes a single additional argument x, where x must be a positive\n" \
	"real number. The time step of the integration performed in the\n" \
	"simulation will be set to x."
#define network_callback_desc \
	"The callback function to use for defining the adjacency matrix that\n" \
	"describes the structure of the neural network. The available functions\n" \
	"are listed further in this document."
#define neuron_callback_desc \
	"The callback function to use for defining the initial conditions and\n" \
	"parameter set of each individual neuron in the neural network. The\n" \
	"available functions are listed further in this document."
#define screen_width_desc "The screen width in pixels."
#define screen_height_desc "The screen height in pixels."
#define fontpath_desc "The relative path to the TrueType font to use."
#define final_time_desc "The termination time of the simulation in milliseconds."
#define print_time_desc \
	"Output a data point every time this duration passes, relative\n" \
	"to the simulation time."
#define output_dir_desc  "The name of the directory to output the files to."
#define print_neurons_desc "Toggle for printing the neuron data files individually."
#define print_voltage_matrix_desc  "Toggle for printing the voltage matrix file."

static const struct neuron_init_entry neuron_init_entries[] = {
	neuron_init_entry("tonic",
			  neuron_init_callback_tonic,
			  "All neurons have the tonic profile."),
	neuron_init_entry("bursting",
			  neuron_init_callback_bursting,
			  "All neurons have the bursting profile."),
	neuron_init_entry("single-center",
			  neuron_init_callback_single_center,
			  "Center is tonic, rest are bursting."),
	neuron_init_entry("double-center",
			  neuron_init_callback_double_center,
			  "Two tonic neurons in center, rest are bursting."),
	neuron_init_null_entry
};

static const struct adj_matrix_init_entry adj_matrix_init_entries[] = {
	adj_matrix_init_entry("empty",
			      adj_matrix_init_callback_empty,
			      "The graph describing the coupling has no edges."),
	adj_matrix_init_entry("complete",
			      adj_matrix_init_callback_complete,
			      "The graph describing the coupling is a complete graph."),
	adj_matrix_init_entry("lattice",
			      adj_matrix_init_callback_lattice,
			      "Neurons are coupled with their four nearest neighbors."),
	adj_matrix_init_null_entry
};

struct run_state {
	enum RunStateType {
		RUN_STATE_VISUALIZE,
		RUN_STATE_OUTPUT_DATA,
		RUN_STATE_PRINT_VERSION,
		RUN_STATE_PRINT_HELP,
		RUN_STATE_ERROR,
	} type;
	struct simulation_options {
		bool coupling_constant_is_random;
		double coupling_constant;
		struct random_value_interval {
			double highest;
			double lowest;
		} random_value_interval;
		uint neuron_count;
		double time_step;
		double (*network_callback)(uint size, uint row, uint col);
		void (*neuron_callback)(uint i, uint count,
				    double *V, double *a_K, double *a_sd, double *a_sr,
				    struct neuron_profile **np);	
	} simopts;
	struct print_options {
		double final_time;
		double print_time;
		const char *output_directory;
		bool print_neurons;
		bool print_voltage_matrix;
	} popts;
	struct visual_options {
		uint screen_width;
		uint screen_height;
		const char *fontpath;
	} vopts;
};

bool parse_help(const char ***args, struct run_state *rs)
{
	rs->type = RUN_STATE_PRINT_HELP;
	*args += 1;
	return true;
}

bool parse_version(const char ***args, struct run_state *rs)
{
	rs->type = RUN_STATE_PRINT_VERSION;
	*args += 1;
	return true;
}

bool parse_visualize(const char ***args, struct run_state *rs)
{
	rs->type = RUN_STATE_VISUALIZE;
	*args += 1;
	return true;
}

bool parse_output_data(const char ***args, struct run_state *rs)
{
	rs->type = RUN_STATE_OUTPUT_DATA;
	*args += 1;
	return true;
}

bool parse_random_coupling(const char ***args, struct run_state *rs)
{
	/* parse two real numbers between 0.0 and 1.0 */
	const char *lowstr = (*args)[1];
	if (!lowstr) {
		return false;
	}
	const char *highstr = (*args)[2];
	if (!highstr) {
		return false;
	}
	char *endlow, *endhigh;
	double low = strtod(lowstr, &endlow);
	double high = strtod(lowstr, &endhigh);
	if (!(*endlow == '\0' && *endhigh == '\0')) {
		return false;
	}
	if (low >= high || low < 0.0 || low > 1.0 || high < 0.0 || high > 1.0) {
		return false;
	}

	rs->simopts.coupling_constant_is_random = true;
	rs->simopts.random_value_interval.highest = high;
	rs->simopts.random_value_interval.lowest = low;

	*args += 3;
	return true;
}

bool parse_coupling_constant(const char ***args, struct run_state *rs)
{
	/* parse one real number between 0.0 and 1.0 */
	const char *coupling_str = (*args)[1];
	if (!coupling_str) {
		return false;
	}
	char *end;
	double coupling_constant = strtod(coupling_str, &end);
	if (*end != '\0') {
		return false;
	}

	if (coupling_constant < 0.0 || coupling_constant > 1.0) {
		return false;
	}
	
	rs->simopts.coupling_constant = coupling_constant;
	rs->simopts.coupling_constant_is_random = false;

	*args += 2;
	return true;
}

bool parse_neuron_count(const char ***args, struct run_state *rs)
{
	/* parse one positive integer */
	const char *neuron_count_str = (*args)[1];
	if (!neuron_count_str) {
		return false;
	}
	char *end;
	long neuron_count = strtol(neuron_count_str, &end, 10);
	if (*end != '\0') {
		return false;
	}

	if (neuron_count < 0) {
		return false;
	}
	
	rs->simopts.neuron_count = (uint)neuron_count;
	*args += 2;
	return true;	
}

bool parse_time_step(const char ***args, struct run_state *rs)
{
	/* parse one positive real number */
	const char *time_step_str = (*args)[1];
	if (!time_step_str) {
		return false;
	}
	char *end;
	double time_step = strtod(time_step_str, &end);
	if (*end != '\0') {
		return false;
	}

	if (time_step < 0.0) {
		return false;
	}

	rs->simopts.time_step = time_step;
	*args += 2;
	return true;
}

bool parse_network_callback(const char ***args, struct run_state *rs)
{
	/* look for symbol in adj_matrix_init_entries */
	const char *network_callback_name = (*args)[1];
	if (!network_callback_name) {
		return false;
	}
	for (const struct adj_matrix_init_entry *a = adj_matrix_init_entries; !is_adj_matrix_init_entry_empty(*a); a++) {
		if (!strcmp(a->name, network_callback_name)) {
			rs->simopts.network_callback = a->callback;
			*args += 2;
			return true;
		}
	}

	return false;	
}

bool parse_neuron_callback(const char ***args, struct run_state *rs)
{
	/* look for symbol in neuron_init_entries */
	const char *neuron_callback_name = (*args)[1];
	if (!neuron_callback_name) {
		return false;
	}
	for (const struct neuron_init_entry *n = neuron_init_entries; !is_neuron_init_entry_empty(*n); n++) {
		if (!strcmp(n->name, neuron_callback_name)) {
			rs->simopts.neuron_callback = n->callback;
			*args += 2;
			return true;
		}
	}

	return false;	
}

bool parse_screen_width(const char ***args, struct run_state *rs)
{
	/* parse one positive integer */
	const char *screen_width_str = (*args)[1];
	if (!screen_width_str) {
		return false;
	}
	char *end;
	long screen_width = strtol(screen_width_str, &end, 10);
	if (*end != '\0') {
		return false;
	}

	if (screen_width < 0.0) {
		return false;
	}

	rs->vopts.screen_width = screen_width;
	*args += 2;
	return true;
}

bool parse_screen_height(const char ***args, struct run_state *rs)
{
	/* parse one positive integer */
	const char *screen_height_str = (*args)[1];
	if (!screen_height_str) {
		return false;
	}
	char *end;
	long screen_height = strtol(screen_height_str, &end, 10);
	if (*end != '\0') {
		return false;
	}

	if (screen_height < 0.0) {
		return false;
	}

	rs->vopts.screen_height = screen_height;
	*args += 2;
	return true;
}

bool parse_fontpath(const char ***args, struct run_state *rs)
{
	/* parse one string */
	const char *fontpath = (*args)[1];
	if (!fontpath) {
		return false;
	}

	rs->vopts.fontpath = fontpath;
	*args += 2;
	return true;
}

bool parse_final_time(const char ***args, struct run_state *rs)
{
	/* parse one positive real number */
	const char *final_time_str = (*args)[1];
	if (!final_time_str) {
		return false;
	}
	char *end;
	double final_time = strtod(final_time_str, &end);
	if (*end != '\0') {
		return false;
	}

	if (final_time < 0.0) {
		return false;
	}

	rs->popts.final_time = final_time;
	*args += 2;
	return true;
}

bool parse_print_time(const char ***args, struct run_state *rs)
{
	/* parse one positive real number */
	const char *print_time_str = (*args)[1];
	if (!print_time_str) {
		return false;
	}
	char *end;
	double print_time = strtod(print_time_str, &end);
	if (*end != '\0') {
		return false;
	}

	if (print_time < 0.0) {
		return false;
	}

	rs->popts.print_time = print_time;
	*args += 2;
	return true;
}

bool parse_output_dir(const char ***args, struct run_state *rs) {
     	/* parse one string */
	const char *output_directory = (*args)[1];
	if (!output_directory) {
		return false;
	}

	rs->popts.output_directory = output_directory;
	*args += 2;
	return true;	
}

bool parse_print_neurons(const char ***args, struct run_state *rs) {
	/* parse one boolean */
	const char *print_neurons_str = (*args)[1];
	if (!print_neurons_str) {
		return false;
	}

	bool print_neurons;
	if (!strcmp(print_neurons_str, "true")) {
		print_neurons = true;
	}
	else if (!strcmp(print_neurons_str, "false")) {
		print_neurons = false;
	}
	else {
		return false;
	}

	rs->popts.print_neurons = print_neurons;
	*args += 2;
	return true;
}

bool parse_print_voltage_matrix(const char ***args, struct run_state *rs) {
	/* parse one boolean */
	const char *print_voltage_matrix_str = (*args)[1];
	if (!print_voltage_matrix_str) {
		return false;
	}

	bool print_voltage_matrix;
	if (!strcmp(print_voltage_matrix_str, "true")) {
		print_voltage_matrix = true;
	}
	else if (!strcmp(print_voltage_matrix_str, "false")) {
		print_voltage_matrix = false;
	}
	else {
		return false;
	}

	rs->popts.print_voltage_matrix = print_voltage_matrix;
	*args += 2;
	return true;
}

struct command_line_option {
	const char *option;
	const char *desc;
	bool (*parser)(const char ***args, struct run_state *rs);
};

#define command_line_entry(option_name, option_parser, option_desc)	\
	(struct command_line_option) {\
		.option = option_name,\
		.parser = option_parser,\
		.desc = option_desc\
	}

#define command_line_null_entry (struct command_line_option) \
	(struct command_line_option) {\
		.option = (void *)0,\
		.parser = (void *)0,\
		.desc = (void *)0\
	}

bool is_command_line_entry_empty(struct command_line_option e) {
	return e.option == (void *)0 && e.parser == (void *)0;
}

const struct command_line_option command_line_entries[] = {
	command_line_entry("-h", &parse_help, help_desc),
	command_line_entry("--help", &parse_help, help_desc),
	command_line_entry("-v", &parse_version, version_desc),
	command_line_entry("--version", &parse_version, version_desc),
	command_line_entry("--visualize", &parse_visualize, visualize_desc),
	command_line_entry("--output-data", &parse_output_data, output_data_desc),
	command_line_entry("--random-coupling", &parse_random_coupling, random_coupling_desc),
	command_line_entry("--coupling-constant", &parse_coupling_constant, coupling_constant_desc),
	command_line_entry("--neuron-count", &parse_neuron_count, neuron_count_desc),
	command_line_entry("--time-step", &parse_time_step, time_step_desc),
	command_line_entry("--network-callback", &parse_network_callback, network_callback_desc),
	command_line_entry("--neuron-callback", &parse_neuron_callback, neuron_callback_desc),
	command_line_entry("--screen-width", &parse_screen_width, screen_width_desc),
	command_line_entry("--screen-height", &parse_screen_height, screen_height_desc),
	command_line_entry("--fontpath", &parse_fontpath, fontpath_desc),
	command_line_entry("--final-time", &parse_final_time, final_time_desc),
	command_line_entry("--print-time", &parse_print_time, print_time_desc),
	command_line_entry("--output-dir", &parse_output_dir, output_dir_desc),
	command_line_entry("--print-neurons", &parse_print_neurons, print_neurons_desc),
	command_line_entry("--print-voltage-matrix", &parse_print_voltage_matrix, print_voltage_matrix_desc),
	command_line_null_entry
};

static const struct run_state default_run_state = (const struct run_state) {
	.type = RUN_STATE_ERROR,
	.simopts.coupling_constant_is_random = false,
	.simopts.coupling_constant = 0.1,
	.simopts.random_value_interval.highest = 0.0,
	.simopts.random_value_interval.lowest = 0.0,
	.simopts.neuron_count = 225,
	.simopts.time_step = 0.1,
	.simopts.network_callback = &adj_matrix_init_callback_lattice,
	.simopts.neuron_callback = &neuron_init_callback_single_center,
	.popts.final_time = 10000,
	.popts.print_time = 1,
	.popts.output_directory = "output",
	.popts.print_neurons = false,
	.popts.print_voltage_matrix = true,
	.vopts.screen_width = 800,
	.vopts.screen_height = 800,
	.vopts.fontpath = "res/LiberationMono-Regular.ttf"
};

int print_help(void);
int print_version(void);
int visualize_main(struct visual_options options);
int print_data_main(struct print_options options);

struct run_state parse_command_line_arguments(int argc, const char **argv)
{
	struct run_state result = default_run_state;
	
	if (argc == 1) {
		result.type = RUN_STATE_ERROR;
	}
	else {
		argv = &argv[1]; /* get rid of the program name */
		while (*argv != NULL) {
			const char *current_arg = argv[0];
			for (const struct command_line_option *o = command_line_entries;
			     !is_command_line_entry_empty(*o);
			     o++) {
				if (!strcmp(o->option, current_arg)) {
					if (!o->parser(&argv, &result)) {
						result.type = RUN_STATE_ERROR;
						return result;
					}
				}
			}
			if (current_arg == argv[0]) { /* no parsing occurred, unknown arg */
				result.type = RUN_STATE_ERROR;
				return result;
			}
		}
	}

	return result;
}

int main(int argc, const char **argv)
{
	struct run_state state = parse_command_line_arguments(argc, argv);

	switch(state.type) {
	case RUN_STATE_VISUALIZE:
		return visualize_main(state.vopts);
	case RUN_STATE_OUTPUT_DATA:
		return print_data_main(state.popts);
	case RUN_STATE_PRINT_VERSION:
		return print_version();
	case RUN_STATE_PRINT_HELP:
		return print_help();
	case RUN_STATE_ERROR:
		puts("Invalid command line arguments used. Use 'neuralnet -h' for help.");
		break;
	}

	return 1;
}

int visualize_main(struct visual_options options)
{
	uint screen_width = 800;
	uint screen_height = 800;
	uint header_height = 100;
	
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	SDL_Window *window = SDL_CreateWindow("neuralnet", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, 0);
	TTF_Font *font = TTF_OpenFont("res/LiberationMono-Regular.ttf", 24);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	bool running = true;
	uint millis_per_frame = 1000 / 60;
	uint start_time_millis, end_time_millis;
	while (running) {
		start_time_millis = SDL_GetTicks();
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_KEYDOWN: {
				SDL_Scancode scancode = e.key.keysym.scancode;
				if (scancode == SDL_SCANCODE_ESCAPE) {
					running = false;
				}
				else if (scancode == SDL_SCANCODE_Q) {
					running = false;
				}
				else if (scancode == SDL_SCANCODE_UP) {
					
				}
				else if (scancode == SDL_SCANCODE_DOWN) {
					
				}
				else if (scancode == SDL_SCANCODE_LEFT) {
					
				}
				else if (scancode == SDL_SCANCODE_RIGHT) {
					
				}
				break;
			}
			case SDL_QUIT:
				running = false;
				break;
			}
		}
		SDL_SetRenderTarget(renderer, NULL);
		SDL_SetRenderDrawColor(renderer, 220, 210, 190, 255);
		SDL_RenderClear(renderer);
		
		/* draw the heat map cells representing the voltage of each neuron */
		/* draw header information */

		SDL_RenderPresent(renderer);
		end_time_millis = SDL_GetTicks();
		uint duration_millis = start_time_millis - end_time_millis;
		if (duration_millis > millis_per_frame) {
			SDL_Delay(millis_per_frame - duration_millis);
		}
	}

	SDL_DestroyRenderer(renderer);
	TTF_CloseFont(font);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();

	return 0;
}

int print_data_main(struct print_options options)
{
	const uint lattice_width = 3;
	const uint lattice_height = 3;
	const uint neuron_count = lattice_width * lattice_height;
	const double time_step = 0.1;
	const double final_time = 5000;
	const double print_time = 1.0;
	const double progress_print_interval = 1.0;
	
	file_table fs = file_table_create("output", neuron_count, 1, "voltage_matrix.dat");
	if (!fs) {
		puts("Fatal error: Could not create the file table.");
		return 1;
	}
	
	adj_matrix am = adj_matrix_create(neuron_count);
	adj_matrix_set_custom(am, &adj_matrix_init_callback_lattice);
	neural_network ns = neural_network_create(neuron_count, &neuron_init_callback_single_center, am);

	timer timer = timer_begin();

	double sim_time;
	
	while ((sim_time = neural_network_get_time(ns)) < final_time) {
		timer_print(timer, progress_print_interval,
				    "Progress: %3d%%, Time elapsed: %9.2fs\n",
				    (int)(100 * sim_time / final_time),
				    timer_total_get(timer));
		if (math_utils_near_every(sim_time, time_step, 1.0)) {
			if (options.print_neurons) {
				for (uint i = 0; i < neural_network_get_count(ns); i++) {	
					file_table_index_print(fs, i, "%.10e %.10e %.10e %.10e %.10e\n",
							       neural_network_get_time(ns),
							       neural_network_get_V(ns, i),
							       neural_network_get_a_K(ns, i),
							       neural_network_get_a_sd(ns, i),
							       neural_network_get_a_sr(ns, i));
				}
			}
			if (options.print_voltage_matrix) {
				file_table_special_print(fs, "voltage_matrix.dat", "#aside time = %f ms\n", sim_time);
				for (uint row = 0; row < lattice_height; row++) {
					for (uint col = 0; col < lattice_width; col++) {
						file_table_special_print(fs, "voltage_matrix.dat", "%.10e ", neural_network_get_V(ns, row * lattice_width + col));
					}
					file_table_special_print(fs, "voltage_matrix.dat", "\n");
				}
				file_table_special_print(fs, "voltage_matrix.dat", "\n");
			}
		}
		
		neural_network_integrate(ns, time_step);
	}

	timer_end(&timer, "Total elapsed time: %.2fs\n", timer_total_get(timer));
	
	adj_matrix_destroy(&am);
	neural_network_destroy(&ns);
	file_table_destroy(&fs);
	
	return 0;
}

int print_version(void)
{
	puts(version_statement);
	return 0;
}

int print_help(void)
{
	puts(version_statement);
	puts(hrule);
	puts("Listed below are the command line options that are available.");
	puts(hrule);
	for (const struct command_line_option *o = command_line_entries;
	     !is_command_line_entry_empty(*o);
	     o++) {
		printf("%s\n%s\n\n", o->option, o->desc);
	}
	puts(hrule);
	puts("Available values to use with the \"--network-callback\" option:");
	puts(hrule);
       for (const struct adj_matrix_init_entry *a = adj_matrix_init_entries; !is_adj_matrix_init_entry_empty(*a); a++) {
	       printf("%s\n%s\n\n", a->name, a->desc);
       }
	puts(hrule);
	puts("Available values to use with the \"--neuron-callback\" option:");
	puts(hrule);
	for (const struct neuron_init_entry *n = neuron_init_entries; !is_neuron_init_entry_empty(*n); n++) {
	       printf("%s\n%s\n\n", n->name, n->desc);
	}
	
	return 0;
}
