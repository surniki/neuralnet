
#include "headers/temp_memory.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void *temp_memory;
static size_t current_size;
static bool allocated = false;

void *temp_malloc(size_t size)
{
	assert(!allocated);
	
	if (size > current_size || !temp_memory) {
		current_size = size;
		temp_memory = realloc(temp_memory, current_size);
	}

	memset(temp_memory, 0, current_size);
	return temp_memory;
}

void temp_release()
{
	allocated = false;
}

void temp_free()
{
	free(temp_memory);
	temp_memory = NULL;
	current_size = 0;
	allocated = false;
}
