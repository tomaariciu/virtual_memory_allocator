#include "utils.h"

void initialise_info(general_info_t *info) {
	info->total_memory = 0;
	info->allocated_memory = 0;
	info->free_memory = 0;
	info->free_blocks = 0;
	info->allocated_blocks = 0;
	info->malloc_calls = 0;
	info->free_calls = 0;
	info->fragmentations = 0;
	info->reconstruction = 0;
}