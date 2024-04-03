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
	doubly_linked_list_t *allocated_blocks = create_dll();
	do {
		scanf("%s", command);
		//printf("Comanda este %s\n", command);
		end_program = parse_command(command, &info, &segregated_free_lists, allocated_blocks);
	} while (!end_program);
	return 0;
}