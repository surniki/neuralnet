
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include "headers/deftypes.h"
#include "headers/file_array.h"

#include "tests/headers/test_utils.h"

struct file_array {
	uint length;
	FILE *files[];
};

file_array file_array_create(uint length, const char *output_dir_name)
{
	file_array result = malloc((sizeof *result) + (sizeof *(result->files)) * length);
	result->length = length;

	long long digits = (long long)ceil(log(length)) + 1;
	uint filename_size = strlen(output_dir_name) + strlen("/.dat") + 1 + digits;
	char *filename = malloc(filename_size);
	for (uint i = 0; i < length; i++) {
		snprintf(filename, filename_size, "%s/%d.dat", output_dir_name, i);
		result->files[i] = fopen(filename, "+w");
	}

	free(filename);
	return result;
}

void file_array_print(file_array fs, uint i, const char *fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	vfprintf(fs->files[i], fmt, vl);
	va_end(vl);
}

void file_array_destroy(file_array *fs)
{
	for (uint i = 0; i < (*fs)->length; i++) {
		fclose((*fs)->files[i]);
	}

	free(*fs);
	*fs = NULL;
}
