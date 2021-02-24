
#ifndef FILE_TABLE_H
#define FILE_TABLE_H

#include "deftypes.h"

struct file_table;
typedef struct file_table *file_table;

file_table file_table_create(const char *dirname, uint length, uint special_count, ...);
uint file_table_get_length(file_table fs);
uint file_table_get_special_count(file_table fs);
void file_table_index_print(file_table fs, uint i, const char *fmt, ...);
void file_table_special_print(file_table fs, const char *name, const char *fmt, ...);
void file_table_destroy(file_table *fs);

#endif
