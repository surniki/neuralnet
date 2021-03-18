
#ifndef TEMP_MEMORY_H
#define TEMP_MEMORY_H

#include <stdbool.h>
#include <stdlib.h>

void *temp_malloc(size_t size);
void temp_release(void);
void temp_free(void);

#endif
