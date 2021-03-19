
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
#include "headers/math_utils.h"
#include "headers/neuron_config.h"
#include "headers/dynamical_system.h"
#include "headers/temp_memory.h"

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


struct data_entry {
	const char *name, *desc;
	void *data;
};
       
static const struct data_entry parameter_callback_entries[] = {
	(struct data_entry) {
		.name = "huber-braun-double-center",
		.desc = "Two tonic neurons in center, rest are bursting.",
		.data = &huber_braun_parameter_callback_double_center
	},
	(struct data_entry) {
		.name = "huber-braun-single-center",
		.desc = "Center is tonic, rest are bursting.",
		.data = &huber_braun_parameter_callback_single_center
	},
	(struct data_entry) {
		.name = "huber-braun-tonic",
		.desc = "All neurons have the tonic profile.",
		.data = &huber_braun_parameter_callback_tonic
	},
	(struct data_entry) {
		.name = "huber-braun-bursting",
		.desc = "All neurons have the bursting profile.",
		.data = &huber_braun_parameter_callback_bursting
	},
	(struct data_entry) {0}
};

static const struct data_entry coupling_callback_entries[] = {
	(struct data_entry) {
		.name = "empty",
		.desc = "The graph describing the coupling has no edges.",
		.data = &coupling_callback_empty
	},
	(struct data_entry) {
		.name = "complete",
		.desc = "The graph describing the coupling is a complete graph.",
		.data = &coupling_callback_complete
	},
	(struct data_entry) {
		.name = "lattice",
		.desc = "Neurons are coupled with their four nearest neighbors.",
		.data = &coupling_callback_lattice
	},
	(struct data_entry) {0}	
};

static const struct data_entry initial_values_callback_entries[] = {
	(struct data_entry) {
		.name = "zero",
		.desc = "Sets all dynamical variables to be zero at the start.",
		.data = &initial_values_callback_zero
	},
	(struct data_entry) {0}
};

static const struct data_entry model_entries[] = {
	(struct data_entry) {
		.name = "huber-braun",
		.desc = "The Huber-Braun neuron model.",
		.data = &huber_braun_model
	},
	(struct data_entry) {0}
};
	
bool is_entry_empty(const struct data_entry *e)
{
	return e->name == (void *)0 && e->desc == (void *)0 && e->data == (void *)0;
}

#define for_entries(entry, entries) \
	for (const struct data_entry *entry = entries; !is_entry_empty(entry); entry++)

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
		void *(*parameter_callback)(dynamical_system, uint);
		uint (*coupling_callback)(dynamical_system, uint);
		void (*initial_values_callback)(uint, uint, double *);
		struct dynamical_model *model;
	} simopts;
	struct print_options {
		double final_time;
		double print_time;
		const char *output_dir;
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
	const char *first_str = (*args)[1];
	if (!first_str) {
		return false;
	}
	const char *second_str = (*args)[2];
	if (!second_str) {
		return false;
	}
	char *end_first, *end_second;
	double first = strtod(first_str, &end_first);
	double second = strtod(second_str, &end_second);
       	double low = (first < second) ? first : second;
	double high = (first > second) ? first : second;

	if (*end_first != '\0' || *end_second != '\0') {
		return false;
	}
	if (low < 0.0 || low > 1.0 || high < 0.0 || high > 1.0) {
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

bool parse_parameter_callback(const char ***args, struct run_state *rs)
{
	/* look for symbol in parameter_callback_entries */
	const char *parameter_callback_name = (*args)[1];
	if (!parameter_callback_name) {
		return false;
	}
	for_entries(entry, parameter_callback_entries) {
		if (!strcmp(entry->name, parameter_callback_name)) {
			rs->simopts.parameter_callback = entry->data;
			*args += 2;
			return true;
		}
	}

	return false;
}

bool parse_coupling_callback(const char ***args, struct run_state *rs)
{
	/* look for symbol in coupling_callback_entries */
	const char *coupling_callback_name = (*args)[1];
	if (!coupling_callback_name) {
		return false;
	}
	for_entries(entry, coupling_callback_entries) {
		if (!strcmp(entry->name, coupling_callback_name)) {
			rs->simopts.coupling_callback = entry->data;
			*args += 2;
			return true;
		}
	}

	return false;
}

bool parse_initial_values_callback(const char ***args, struct run_state *rs)
{
	/* look for symbol in initial_value_callback_entries */
	const char *initial_values_callback_name = (*args)[1];
	if (!initial_values_callback_name) {
		return false;
	}
	for_entries(entry, initial_values_callback_entries) {
		if (!strcmp(entry->name, initial_values_callback_name)) {
			rs->simopts.initial_values_callback = entry->data;
			*args += 2;
			return true;
		}
	}

	return false;
}

bool parse_model(const char ***args, struct run_state *rs)
{
	/* look for symbol in model_entries */
	const char *model_name = (*args)[1];
	if (!model_name) {
		return false;
	}
	for_entries(entry, model_entries) {
		if (!strcmp(entry->name, model_name)) {
			rs->simopts.model = entry->data;
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
	const char *output_dir = (*args)[1];
	if (!output_dir) {
		return false;
	}

	rs->popts.output_dir = output_dir;
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

bool is_option_empty(const struct command_line_option *e) {
	return e->option == (void *)0 && e->desc == (void *)0 && e->parser == (void *)0;
}

#define for_options(option, options) \
	for(const struct command_line_option *option = options; !is_option_empty(option); option++)

const struct command_line_option command_line_options[] = {
	(struct command_line_option) {
		.option = "-h",
		.parser = &parse_help,
		.desc = help_desc
	},
	(struct command_line_option) {
		.option = "--help",
		.parser = &parse_help,
		.desc = help_desc
	},
	(struct command_line_option) {
		.option = "-v",
		.parser = &parse_version,
		.desc = version_desc
	},
	(struct command_line_option) {
		.option = "--version",
		.parser = &parse_version,
		.desc = version_desc
	},
	(struct command_line_option) {
		.option = "--visualize",
		.parser = &parse_visualize,
		.desc = visualize_desc
	},
	(struct command_line_option) {
		.option = "--output-data",
		.parser = &parse_output_data,
		.desc = output_data_desc
	},
	(struct command_line_option) {
		.option = "--random-coupling",
		.parser = &parse_random_coupling,
		.desc = random_coupling_desc
	},
	(struct command_line_option) {
		.option = "--coupling-constant",
		.parser = &parse_coupling_constant,
		.desc = coupling_constant_desc
	},
	(struct command_line_option) {
		.option = "--neuron-count",
		.parser = &parse_neuron_count,
		.desc = neuron_count_desc
	},
	(struct command_line_option) {
		.option = "--time-step",
		.parser = &parse_time_step,
		.desc = time_step_desc
	},
	(struct command_line_option) {
		.option = "--parameter-callback",
		.parser = &parse_parameter_callback,
		.desc = "TODO: Add description."
	},
	(struct command_line_option) {
		.option = "--coupling-callback",
		.parser = &parse_coupling_callback,
		.desc = "TODO: Add description."
	},
	(struct command_line_option) {
		.option = "--initial-values-callback",
		.parser = &parse_initial_values_callback,
		.desc = "TODO: Add description."
	},
	(struct command_line_option) {
		.option = "--model",
		.parser = &parse_model,
		.desc = "TODO: Add description."
	},
	(struct command_line_option) {
		.option = "--screen-width",
		.parser = &parse_screen_width,
		.desc = screen_width_desc
	},
	(struct command_line_option) {
		.option = "--screen-height",
		.parser = &parse_screen_height,
		.desc = screen_height_desc
	},
	(struct command_line_option) {
		.option = "--font",
		.parser = &parse_fontpath,
		.desc = fontpath_desc
	},
	(struct command_line_option) {
		.option = "--final-time",
		.parser = &parse_final_time,
		.desc = final_time_desc
	},
	(struct command_line_option) {
		.option = "--print-time",
		.parser = &parse_print_time,
		.desc = print_time_desc
	},
	(struct command_line_option) {
		.option = "--output-dir",
		.parser = &parse_output_dir,
		.desc = output_dir_desc
	},
	(struct command_line_option) {
		.option = "--print-neurons",
		.parser = &parse_print_neurons,
		.desc = print_neurons_desc
	},
	(struct command_line_option) {
		.option = "--print-voltage-matrix",
		.parser = &parse_print_voltage_matrix,
		.desc = print_voltage_matrix_desc
	},
	(struct command_line_option) {0}
};

static const struct run_state default_run_state = (const struct run_state) {
	.type = RUN_STATE_ERROR,
	.simopts.coupling_constant_is_random = false,
	.simopts.coupling_constant = 0.1,
	.simopts.random_value_interval.highest = 0.0,
	.simopts.random_value_interval.lowest = 0.0,
	.simopts.neuron_count = 225,
	.simopts.time_step = 0.1,
	.simopts.parameter_callback = &huber_braun_parameter_callback_single_center,
	.simopts.coupling_callback = &coupling_callback_lattice,
	.simopts.initial_values_callback = &initial_values_callback_zero,
	.simopts.model = &huber_braun_model,
	.popts.final_time = 10000,
	.popts.print_time = 1,
	.popts.output_dir = "output",
	.popts.print_neurons = false,
	.popts.print_voltage_matrix = true,
	.vopts.screen_width = 800,
	.vopts.screen_height = 800,
	.vopts.fontpath = "res/Monoid-Regular-NoCalt.ttf"
};

int print_help(void);
int print_version(void);
int visualize_main(struct simulation_options *simopts, struct visual_options *vopts);
int print_data_main(struct simulation_options *simopts, struct print_options *popts);

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
			for_options(o, command_line_options) {
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
		return visualize_main(&state.simopts, &state.vopts);
	case RUN_STATE_OUTPUT_DATA:
		return print_data_main(&state.simopts, &state.popts);
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

const double low_voltage = -80.0;
const double high_voltage = 20.0;
const char *low_voltage_text = "-80.0";
const char *high_voltage_text = "20.0";

void clerp(double input, double low_input, double high_input, SDL_Color *low_color, SDL_Color *high_color, SDL_Color *out_color)
{
	out_color->r = math_utils_lerp(input, low_input, high_input, low_color->r, high_color->r);
	out_color->g = math_utils_lerp(input, low_input, high_input, low_color->g, high_color->g);
	out_color->b = math_utils_lerp(input, low_input, high_input, low_color->b, high_color->b);
	out_color->a = 255;
}

void draw_color_map_cell(SDL_Renderer *renderer,
			 SDL_Color *low_color, SDL_Color *high_color,
			 uint x, uint y, uint width, uint height, double value)
{
	SDL_Color c;
	clerp(value, low_voltage, high_voltage, low_color, high_color, &c);

	SDL_Rect rect = (SDL_Rect) {
		.x = x,
		.y = y,
		.w = width,
		.h = height
	};

	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
	SDL_RenderFillRect(renderer, &rect);
}

void draw_legend(SDL_Renderer *renderer, TTF_Font *font,
		 SDL_Color *low_color, SDL_Color *high_color,
		 uint x, uint y, uint width, uint height)
{
	SDL_Surface *low_text_surface = TTF_RenderText_Solid(font, low_voltage_text, (SDL_Color){80, 80, 80, 255});
	SDL_Surface *high_text_surface = TTF_RenderText_Solid(font, high_voltage_text, (SDL_Color){80, 80, 80, 255});
	uint low_width = low_text_surface->w;
	uint low_height = low_text_surface->h;
	uint high_width = high_text_surface->w;
	uint high_height = high_text_surface->h;
	SDL_Texture *low_text_texture = SDL_CreateTextureFromSurface(renderer, low_text_surface);
	SDL_Texture *high_text_texture = SDL_CreateTextureFromSurface(renderer, high_text_surface);
	uint padding_width = 10;
	uint color_bar_width = width - (low_width + high_width) - 2 * padding_width;
	uint color_bar_height = height;
	uint color_bar_x = x + low_width + padding_width;
	uint color_bar_y = y;

	SDL_Rect low_text_rect;
	low_text_rect.x = x + padding_width / 2.0;
	low_text_rect.y = (y + color_bar_height / 2.0) - (low_height / 2.0);
	low_text_rect.w = low_width;
	low_text_rect.h = low_height;

	SDL_Rect high_text_rect;
	high_text_rect.x = color_bar_x + color_bar_width + padding_width / 2.0;
	high_text_rect.y = (y + color_bar_height / 2.0) - (low_height / 2.0);
	high_text_rect.w = high_width;
	high_text_rect.h = high_height;
	
	SDL_RenderCopy(renderer, low_text_texture, NULL, &low_text_rect);
	SDL_RenderCopy(renderer, high_text_texture, NULL, &high_text_rect);

	uint legend_start = color_bar_x;
	uint legend_end = color_bar_x + color_bar_width;
	for (uint x = legend_start; x <= legend_end; x++) {
		SDL_Color c;
		clerp(x, legend_start, legend_end, low_color, high_color, &c);
		SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
		SDL_Rect rect;
		rect.x = x;
		rect.y = color_bar_y;
		rect.w = 1;
		rect.h = color_bar_height;
		SDL_RenderFillRect(renderer, &rect);
	}

	SDL_FreeSurface(low_text_surface);
	SDL_FreeSurface(high_text_surface);
	SDL_DestroyTexture(low_text_texture);
	SDL_DestroyTexture(high_text_texture);
}

int visualize_main(struct simulation_options *simopts, struct visual_options *vopts)
{	
	if (SDL_Init(SDL_INIT_VIDEO)) {
		puts("Unable to initialize video via SDL_Init. Exiting now.");
		return 1;
	}
	if (TTF_Init()) {
		puts("Unable to initialize text loading via TTF_Init. Exiting now.");
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("neuralnet", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, vopts->screen_width, vopts->screen_height, 0);
	if (!window) {
		puts("Unable to create a window using SDL_CreateWindow. Exiting now.");
		return 1;
	}
	
	TTF_Font *font = TTF_OpenFont(vopts->fontpath, 24);
	if (!font) {
		printf("Unable to find the font located at [%s]. Exiting now.\n", vopts->fontpath);
		return 1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		puts("Unable to create accelerated renderer using SDL_CreateRenderer. Exiting now.");
		return 1;
	}
	
	/* constuct the neural network */
	if (simopts->coupling_constant_is_random) {
		neuron_config_coupling_is_random_set(true,
						     simopts->random_value_interval.highest,
						     simopts->random_value_interval.lowest);
	}
	else {
		neuron_config_coupling_constant_set(simopts->coupling_constant);
	}

	uint matrix_width = sqrt(simopts->neuron_count);
	uint matrix_height = sqrt(simopts->neuron_count);

	double sim_time = 0.0;

	dynamical_system ds = dynamical_system_create(simopts->neuron_count,
						      simopts->model->number_of_variables,
						      simopts->parameter_callback,
						      simopts->coupling_callback,
						      simopts->initial_values_callback,
						      simopts->model->derivatives);

	bool running = true;
	uint frame_step = 1;
	uint millis_per_frame = 1000 / 60;
	uint start_time_millis, end_time_millis;

	SDL_Color low_color;
	low_color.r = 122;
	low_color.g = 31;
	low_color.b = 110;

	SDL_Color high_color;
	high_color.r = 184;
	high_color.g = 172;
	high_color.b = 9;

	uint header_height = 100;
	
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
					frame_step++;
				}
				else if (scancode == SDL_SCANCODE_DOWN) {
					if (frame_step > 1) {
						frame_step--;
					}
				}
				else if (scancode == SDL_SCANCODE_LEFT) {
					math_utils_rk4_integrate(ds, frame_step * -simopts->time_step);
				}
				else if (scancode == SDL_SCANCODE_RIGHT) {
					math_utils_rk4_integrate(ds, frame_step * simopts->time_step);
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
		for (uint row = 0; row < matrix_height; row++) {
			for (uint col = 0; col < matrix_width; col++) {
				double cell_width = vopts->screen_width / (double)matrix_width;
				double cell_height = (vopts->screen_height - header_height) / (double)matrix_height;
				draw_color_map_cell(renderer, &low_color, &high_color,
						    ceill(col * cell_width),
						    ceill(header_height + row * cell_height),
						    ceill(cell_width),
						    ceill(cell_height),
						    dynamical_system_get_value(ds, row * matrix_width + col, 0));
						    
			}
		}

		/* draw header information */
		double sim_time = dynamical_system_get_time(ds);
		char message[512];
		snprintf(message, 512, "Time: %.2fms -- Frame Step: %.2fms", sim_time, simopts->time_step * frame_step);
		SDL_Surface *text_surface = TTF_RenderText_Solid(font, message, (SDL_Color){80, 80, 80, 255});
		SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
		uint message_width = text_surface->w;
		uint message_height = text_surface->h;
		SDL_Rect text_rect;
		text_rect.x = 0;
		text_rect.y = 0;
		text_rect.w = message_width;
		text_rect.h = message_height;
		
		SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
		draw_legend(renderer, font, &low_color, &high_color, 0, message_height, 800, 50);
		
		SDL_RenderPresent(renderer);

		SDL_FreeSurface(text_surface);
		SDL_DestroyTexture(text_texture);

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

	dynamical_system_destroy(&ds);
	temp_free();

	return 0;
}

int print_data_main(struct simulation_options *simopts, struct print_options *popts)
{
	const double progress_print_interval = 1.0;

	if (simopts->coupling_constant_is_random) {
		neuron_config_coupling_is_random_set(true,
						     simopts->random_value_interval.highest,
						     simopts->random_value_interval.lowest);
	}
	else {
		neuron_config_coupling_constant_set(simopts->coupling_constant);
	}

	file_table fs;
	if (popts->print_neurons && popts->print_voltage_matrix) {
		fs = file_table_create(popts->output_dir,
						  simopts->neuron_count,
						  1, "voltage_matrix.dat");
	}
	else if (popts->print_neurons) {
		fs = file_table_create(popts->output_dir, simopts->neuron_count, 0);
	}
	else if (popts->print_voltage_matrix) {
		fs = file_table_create(popts->output_dir, 0, 1, "voltage_matrix.dat");
	}
	else {
		puts("Nothing to do.");
		return 1;
	}

	if (!fs) {
		puts("Fatal error: Could not create the file table.");
		return 1;
	}

	dynamical_system ds = dynamical_system_create(simopts->neuron_count,
						      simopts->model->number_of_variables,
						      simopts->parameter_callback,
						      simopts->coupling_callback,
						      simopts->initial_values_callback,
						      simopts->model->derivatives);
						      
						      
	uint grid_width = sqrt(simopts->neuron_count);
	uint grid_height = sqrt(simopts->neuron_count);

	timer timer = timer_begin();

	double sim_time;	
	while ((sim_time = dynamical_system_get_time(ds)) < popts->final_time) {
		timer_print(timer, progress_print_interval,
				    "Progress: %3d%%, Time elapsed: %9.2fs\n",
				    (int)(100 * sim_time / popts->final_time),
				    timer_total_get(timer));
		if (math_utils_near_every(sim_time, simopts->time_step, popts->print_time)) {
			if (popts->print_neurons) {
				for (uint i = 0; i < dynamical_system_get_system_size(ds); i++) {	
					file_table_index_print(fs, i, "%.10e %.10e %.10e %.10e %.10e\n",
							       dynamical_system_get_time(ds),
							       dynamical_system_get_value(ds, i, 0),
							       dynamical_system_get_value(ds, i, 1),
							       dynamical_system_get_value(ds, i, 2),
							       dynamical_system_get_value(ds, i, 3));

				}
			}
			if (popts->print_voltage_matrix) {
				file_table_special_print(fs, "voltage_matrix.dat",
							 "#aside time = %f ms\n", sim_time);
				for (uint row = 0; row < grid_height; row++) {
					for (uint col = 0; col < grid_width; col++) {
						file_table_special_print(fs, "voltage_matrix.dat", "%.10e ",
									 dynamical_system_get_value(ds, row * grid_width + col, 0));
					}
					file_table_special_print(fs, "voltage_matrix.dat", "\n");
				}
				file_table_special_print(fs, "voltage_matrix.dat", "\n");
			}
		}
		
		math_utils_rk4_integrate(ds, simopts->time_step);
	}

	timer_end(&timer, "Total elapsed time: %.2fs\n", timer_total_get(timer));
	
	dynamical_system_destroy(&ds);
	file_table_destroy(&fs);
	temp_free();
	
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
	for_options (o, command_line_options) {
		printf("%s\n%s\n\n", o->option, o->desc);
	}

	puts(hrule);
	puts("Available values to use with the \"--parameter-callback\" option:");
	puts(hrule);
	for_entries (entry, parameter_callback_entries) {
	       printf("%s\n%s\n\n", entry->name, entry->desc);
	}

	puts(hrule);
	puts("Available values to use with the \"--coupling-callback\" option:");
	puts(hrule);
	for_entries (entry, coupling_callback_entries) {
		printf("%s\n%s\n\n", entry->name, entry->desc);
	}

	puts(hrule);
	puts("Available values to use with the \"--initial-values-callback\" option:");
	puts(hrule);
	for_entries (entry, initial_values_callback_entries) {
		printf("%s\n%s\n\n", entry->name, entry->desc);
	}

	puts(hrule);
	puts("Available values to use with the \"--model\" option:");
	puts(hrule);
	for_entries (entry, model_entries) {
		printf("%s\n%s\n\n", entry->name, entry->desc);
	}

	return 0;
}
