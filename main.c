#include <stdio.h>
#include "utils.h"
#include "dll.h"
#include "commands.h"

int main(void)
{
	char command[MAX_STRING_LENGTH];
	int end_program = 0;
	general_info_t info;
	initialise_info(&info);
	dll_array_t segregated_free_lists;
	doubly_linked_list_t allocated_blocks;
	allocated_blocks.head = NULL;
	allocated_blocks.size = 0;
	do {
		scanf("%s", command);
		end_program = parse_command(command, &info, &segregated_free_lists, &allocated_blocks);
	} while (!end_program);
	return 0;
}