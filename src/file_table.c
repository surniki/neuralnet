
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "headers/deftypes.h"
#include "headers/file_table.h"

#include "tests/headers/test_utils.h"

struct file_table {
	uint length;
	uint special_count;
	struct {
		char *name;
		uint index;
	} *special_names;
	FILE *files[];
};

file_table file_table_create(const char *dirname, uint length, uint special_count, ...)
{	
	file_table result = malloc((sizeof *result) +
				   (sizeof *(result->files)) * (length + special_count));
	result->length = length;
	result->special_count = special_count;

	long long digits = (long long)ceil(log10(length));
	uint filename_size = strlen(dirname) + strlen("/.dat") + 1 + 1 + digits;
	char *filename = malloc(filename_size);

	for (uint i = 0; i < length; i++) {
		snprintf(filename, filename_size, "%s/%d.dat", dirname, i);
		result->files[i] = fopen(filename, "w+");
		if (!result->files[i]) {
			free(filename);
			for (uint j = i; j >= 0; j--) {
				fclose(result->files[j]);
			}
			free(result);
			return NULL;
		}
	}

	free(filename);

	if (special_count) {
		va_list vl;
		va_start(vl, special_count);
		result->special_names = malloc((sizeof *(result->special_names)) * special_count);
		uint max_strlen = 0;
		for (uint i = 0; i < special_count; i++) {
			const char *curr_name = va_arg(vl, const char *);
			uint curr_strlen = strlen(curr_name);
			if (curr_strlen > max_strlen) {
				max_strlen = curr_strlen;
			}
			char *curr_name_copy = malloc(curr_strlen + 1);
			strncpy(curr_name_copy, curr_name, curr_strlen + 1);
			
			result->special_names[i].name = curr_name_copy;
			result->special_names[i].index = length + i;
		}

		va_end(vl);

		filename_size = strlen(dirname) + 1 + 1 + max_strlen;
		filename = malloc(filename_size);
		for (uint i = 0; i < special_count; i++) {
			const char *curr_name = result->special_names[i].name;
			memset(filename, '\0', filename_size);
			snprintf(filename, filename_size, "%s/%s", dirname, curr_name);
			FILE *fh = fopen(filename, "w+");
			if (!fh) {
				for (uint j = i; j >= 0; j--) {
					free(result->special_names[j].name);
				}
				for (uint j = i - 1; j >= 0; j--) {
					fclose(result->files[result->length + j]);
				}
				free(result);
				return NULL;
			}
			result->files[result->length + i] = fh;
		}
		free(filename);
	}
	else {
		result->special_names = NULL;
	}
	
	return result;
}

uint file_table_get_length(file_table fs)
{
	return fs->length;
}

uint file_table_get_special_count(file_table fs)
{
	return fs->special_count;
}

void file_table_index_print(file_table fs, uint i, const char *fmt, ...)
{
	assert(fs);
	assert("Given an invalid index." && i >= 0 && i < fs->length);

	va_list vl;
	va_start(vl, fmt);
	vfprintf(fs->files[i], fmt, vl);
	va_end(vl);
}

void file_table_special_print(file_table fs, const char *name, const char *fmt, ...)
{
	assert(fs);
	assert(fs->special_names);

	va_list vl;
	va_start(vl, fmt);

	for (uint i = 0; i < fs->special_count; i++) {
		if (!strcmp(name, fs->special_names[i].name)) {
			vfprintf(fs->files[fs->special_names[i].index], fmt, vl);
			break;
		}
	}

	va_end(vl);
}

void file_table_destroy(file_table *fs)
{
	assert(fs);
	assert(*fs);

	for (uint i = 0; i < (*fs)->length + (*fs)->special_count; i++) {
		fclose((*fs)->files[i]);
	}
	free((*fs)->special_names);
	free(*fs);
	*fs = NULL;
}
