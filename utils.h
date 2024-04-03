#pragma once

#include <stdio.h>
#include <errno.h>

#define DIE(assertion, call_description)		\
	do {										\
		if (assertion) {						\
			fprintf(stderr, "(%s, %d): ",		\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}										\
	} while (0)

#define MAX_STRING_LENGTH 1000

typedef struct general_info_t {
	int total_memory, allocated_memory, free_memory;
	int free_blocks, allocated_blocks;
	int malloc_calls, free_calls, fragmentations;
	int reconstruction;
	int max_size;
} general_info_t;

void initialise_info(general_info_t *info);