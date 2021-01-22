
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/test_utils.h"
#include <assert.h> /* overwrite 'assert' defined in test_utils.h */

void debug_entry_point(FILE *file);

static inline bool is_test_entry_empty(const struct test_entry t)
{
	return t.test == NULL && t.name == NULL;
}

static inline size_t max(size_t n, size_t m)
{
	return (n > m) ? n : m;
}

static void println_row(char c, size_t width)
{
	for (int i = 0; i < width; i++) {
		putc(c, stdout);
	}
	putc('\n', stdout);
}

int test_main(const struct test_entry *tests)
{
	size_t max_name_length = 0;
	size_t test_count = 0;
	size_t space_width = 8;
	const char *passed_msg = "PASSED";
	const char *failed_msg = "FAILED";

	for (const struct test_entry *t = tests; !is_test_entry_empty(*t); t++) {
		size_t current_name_length = strlen(t->name);
		if (max_name_length < current_name_length) {
			max_name_length = current_name_length;
		}
		++test_count;
	}

	size_t max_status_len = max(strlen(passed_msg), strlen(failed_msg));
	size_t print_width = max_name_length + space_width + max_status_len + 1;
	
	printf("Running %d tests:\n", test_count);
	println_row('-', print_width);

	size_t tests_passed = 0;
	for (const struct test_entry *t = tests; !is_test_entry_empty(*t); t++) {
		bool test_result = t->test();
		printf("%*s %*s\n",
		       max_name_length, t->name,
		       space_width + max_status_len, test_result ? passed_msg : failed_msg);
		if (test_result) {
			tests_passed++;
		}
	}

	println_row('-', print_width);	
	printf("(%d/%d) tests passed. %s\n",
	       tests_passed,
	       test_count,
	       tests_passed == test_count ? "Congratulations!" : "Some tests failed.");

	debug_entry_point(stdout);
	
	return tests_passed == test_count ? 0 : 1;
}

jmp_buf *test_get_assert_buf(bool is_setjmp)
{
	static jmp_buf buf;
	static bool on;

	assert((is_setjmp || on) && "assertion was used in test but not caught");
	
	if (is_setjmp) {
		on = true;
		return &buf;
	}
	else {
		on = false;
		return &buf;
	}
}

#undef malloc
#undef calloc
#undef realloc
#undef free

struct AllocInfo {
    void *handle;
    size_t size;
    size_t line;
    const char *filename;
    struct AllocInfo *next;
};

static struct AllocInfo *allocations = NULL;

struct AllocInfo *allocinfo_remove(struct AllocInfo *list, void *handle)
{
    struct AllocInfo *current;
    struct AllocInfo *temp = NULL;

    /* if the head of the list is being removed */
    if (list->handle == handle) {
	temp = list->next;
	free(list);
	return temp;
    }
    /* if the entry to be removed is in the tail of the list */
    else {
	for (current = list; current; current = current->next) {
	    if (current->next && current->next->handle == handle) {
		temp = current->next;
		current->next = current->next->next;
	    }
	}
	
	if (temp)
	    free(temp);

	return list;
    }
}

size_t allocinfo_length(struct AllocInfo *list)
{
    size_t result = 0;
    struct AllocInfo *current;
    for (current = list; current; current = current->next)
	++result;
    
    return result;
}

void remove_entry_from_allocations(void *ptr)
{
    allocations = allocinfo_remove(allocations, ptr);
}

void add_entry_to_allocations(void *ptr, size_t size, int line, const char *filename)
{
    struct AllocInfo *info_about_allocation = malloc(sizeof (struct AllocInfo));
    info_about_allocation->handle = ptr;
    info_about_allocation->size = size;
    info_about_allocation->line = line;
    info_about_allocation->filename = filename;
    info_about_allocation->next = allocations;
    allocations = info_about_allocation;
}

void *malloc_for_debugging(size_t size, int line, const char *filename)
{
    void *result = malloc(size);
    add_entry_to_allocations(result, size, line, filename);        
    return result;
}

void *calloc_for_debugging(size_t quantity, size_t size, int line, const char *filename)
{
    void *result = calloc(quantity, size);
    add_entry_to_allocations(result, size, line, filename);
    return result;
}

void *realloc_for_debugging(void *ptr, size_t size, int line, const char *filename)
{
    void *result = realloc(ptr, size);
    remove_entry_from_allocations(ptr);
    add_entry_to_allocations(result, size, line, filename);
    return result;
}

void free_for_debugging(void *ptr)
{
    remove_entry_from_allocations(ptr);
    free(ptr);
}

void debug_entry_point(FILE *file)
{
    struct AllocInfo *current;
    int counter = 1;
    fprintf(file, "Current allocation handles still active: %d\n",
	    allocinfo_length(allocations));
    if (allocations) {
	for (current = allocations; current; current = current->next) {
	    fprintf(file, "%4d [%p] of size %5d, allocated on line %20s:%4d.\n",
		    counter++,
		    current->handle,
		    current->size,
		    current->filename,
		    current->line);
	}
    }
    else {
	fprintf(file, "Congratulations! All allocations have been freed.\n");
    }
}

size_t current_number_of_allocations(void)
{
    return allocinfo_length(allocations);
}
