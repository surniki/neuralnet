
#ifndef TEST_UTILS_H
#if defined RUN_TESTS
#define TEST_UTILS_H

#include <stdbool.h>
#include <stdlib.h>
#include <setjmp.h>

jmp_buf *test_get_assert_buf(bool is_setjmp);
#undef assert
#define assert(x) do { if(!(x)) longjmp(*test_get_assert_buf(false), 1); } while(0)
#define is_assert_invoked(x) (setjmp(*test_get_assert_buf(true)) || ((x), 0))

void *malloc_for_debugging(size_t size, int line, const char *filename);
void *calloc_for_debugging(size_t quantity, size_t size, int line, const char *filename);
void *realloc_for_debugging(void *ptr, size_t size, int line, const char *filename);
void free_for_debugging(void *ptr);
size_t current_number_of_allocations();
#define malloc(size) malloc_for_debugging((size), __LINE__, __FILE__)
#define calloc(quantity, size) calloc_for_debugging((quantity), (size), __LINE__, __FILE__)
#define realloc(ptr, new_size) realloc_for_debugging((ptr), (new_size), __LINE__, __FILE__)
#define free free_for_debugging

struct test_entry {
	bool (*test)(void);
	const char *name;	
};
#define test_entry(test_name) (struct test_entry){ .test = &test_name, .name = #test_name }
#define null_entry            (struct test_entry){ .test = (void *)0, .name = (void *)0 }

int test_main(const struct test_entry *tests);

#endif
#endif
