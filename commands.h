#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "dll.h"

int parse_command(char command[MAX_STRING_LENGTH], general_info_t *info, dll_array_t *segregated_free_lists, doubly_linked_list_t *allocated_blocks);
void initialise_heap(general_info_t *info, dll_array_t *segregated_free_lists, int start_address, int no_lists, int list_size);
void dump_memory(general_info_t *info, dll_array_t *segregated_free_lists, doubly_linked_list_t *allocated_blocks);
void malloc_command(general_info_t *info, dll_array_t *segregated_free_lists, doubly_linked_list_t *allocated_blocks, int no_bytes);
void free_command(general_info_t *info, dll_array_t *segregated_free_lists, doubly_linked_list_t *allocated_blocks, int address);
void destroy_heap(dll_array_t *segregated_free_lists, doubly_linked_list_t *allocated_blocks);