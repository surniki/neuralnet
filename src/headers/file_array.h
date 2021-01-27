
#ifndef FILE_ARRAY_H
#define FILE_ARRAY_H

#include "deftypes.h"

struct file_array;
typedef struct file_array *file_array;

file_array file_array_create(uint length, const char *dirname);
uint file_array_get_length(file_array fs);
void file_array_print(file_array fs, uint i, const char *fmt, ...);
void file_array_destroy(file_array *fs);

#endif
