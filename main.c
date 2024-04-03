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
	dll_array_t sfl;
	doubly_linked_list_t *alloc_list = create_dll();
	do {
		scanf("%s", command);
		//printf("Comanda este %s\n", command);
		end_program = parse_command(command, &info, &sfl, alloc_list);
	} while (!end_program);
	return 0;
}
