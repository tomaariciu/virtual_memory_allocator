#include "commands.h"

int parse_command(char command[MAX_STRING_LENGTH], general_info_t *info, dll_array_t *segregated_free_lists, doubly_linked_list_t *allocated_blocks)
{
	(void) segregated_free_lists;
	(void) allocated_blocks;
	if (!strcmp(command, "INIT_HEAP")) {
		int start_address, no_lists, list_size;
		scanf("%x %d %d %d\n", &start_address, &no_lists, &list_size, &info->reconstruction);
		//printf("Comanda este INIT_HEAP, avand parametrii: %d %d %d %d\n", start_address, no_lists, list_size, info->reconstruction);
		initialise_heap(info, segregated_free_lists, start_address, no_lists, list_size);
	}

	if (!strcmp(command, "MALLOC")) {
		int no_bytes;
		scanf("%d\n", &no_bytes);
		//printf("Comanda este MALLOC, avand parametrul %d\n", no_bytes);
		malloc_command(info, segregated_free_lists, allocated_blocks, no_bytes);
	}

	if (!strcmp(command, "FREE")) {
		int address;
		scanf("%x\n", &address);
		//printf("Comanda este FREE, avand parametrul %d\n", address);
		if (!address) {
			return 0;
		}
		free_command(info, segregated_free_lists, allocated_blocks, address);
	}

	if (!strcmp(command, "READ")) {
		int address, no_bytes;
		scanf("%x %d\n", &address, &no_bytes);
		int seg_fault = read_command(allocated_blocks, address, no_bytes);
		if (seg_fault) {
			printf("Segmentation fault (core dumped)\n");
			dump_memory(info, segregated_free_lists, allocated_blocks);
			destroy_heap(segregated_free_lists, allocated_blocks);
			return 1;
		}
		//printf("Comanda este READ, avand parametrii: %d %d\n", address, no_bytes);
	}

	if (!strcmp(command, "WRITE")) {
		int address, no_bytes;
		char data[MAX_STRING_LENGTH], aux[MAX_STRING_LENGTH];
		memset(data, 0, MAX_STRING_LENGTH);
		memset(aux, 0, MAX_STRING_LENGTH);
		scanf("%x \"", &address);
		fgets(data, MAX_STRING_LENGTH, stdin);
		int last_quotes = 0, len = (int) strlen(data);
		for (int i = 0; i < len; i++) {
			if (data[i] == '"') {
				last_quotes = i;
			}
		}
		data[last_quotes] = 0;
		for (int i = last_quotes + 1; i < len; i++) {
			aux[i - last_quotes - 1] = data[i];
			data[i] = 0;
		}
		sscanf(aux, "%d", &no_bytes);
		//printf("Comanda este WRITE, avand parametrii: %x %s %d\n", address, data, no_bytes);
		if ((int) strlen(data) < no_bytes) {
			no_bytes = strlen(data);
		}
		int seg_fault = write_command(allocated_blocks, address, data, no_bytes);
		if (seg_fault) {
			printf("Segmentation fault (core dumped)\n");
			dump_memory(info, segregated_free_lists, allocated_blocks);
			destroy_heap(segregated_free_lists, allocated_blocks);
			return 1;
		}
	}

	if (!strcmp(command, "DUMP_MEMORY")) {
		//printf("Comanda chiar este DUMP_MEMORY\n");
		dump_memory(info, segregated_free_lists, allocated_blocks);
	}

	if (!strcmp(command, "DESTROY_HEAP")) {
		destroy_heap(segregated_free_lists, allocated_blocks);
		return 1;
	}

	return 0;
}


void initialise_heap(general_info_t *info, dll_array_t *segregated_free_lists, int start_address, int no_lists, int list_size)
{
	info->total_memory = no_lists * list_size;
	int max_size = (8 << (no_lists - 1));
	segregated_free_lists->size = max_size;
	segregated_free_lists->v = (doubly_linked_list_t **) malloc((max_size + 1) * sizeof(doubly_linked_list_t *));
	DIE(!segregated_free_lists, "Memory allocation failed!\n");
	for (int i = 1; i <= max_size; i++) {
		segregated_free_lists->v[i] = create_dll();
	}
	int id = 0, address = start_address;
	for (int sz = 8; sz <= max_size; sz <<= 1) {
		dll_node_t *last_node = NULL;
		for (int i = 0; i < list_size / sz; i++) {
			void *data = create_data(id, address, sz, 0);
			dll_node_t *new_node = create_node(data);
			if (!segregated_free_lists->v[sz]->head) {
				segregated_free_lists->v[sz]->head = new_node;
			}
			if (last_node) {
				last_node->nxt = new_node;
			}
			new_node->prv = last_node;
			segregated_free_lists->v[sz]->size++;
			id++;
			address += sz;
			info->free_memory += sz;
			info->free_blocks++;
			last_node = new_node;
		}
	}
}

void dump_memory(general_info_t *info, dll_array_t *segregated_free_lists, doubly_linked_list_t *allocated_blocks)
{
	printf("+++++DUMP+++++\n");
	printf("Total memory: %d bytes\n", info->total_memory);
	printf("Total allocated memory: %d bytes\n", info->allocated_memory);
	printf("Total free memory: %d bytes\n", info->free_memory);
	printf("Free blocks: %d\n", info->free_blocks);
	printf("Number of allocated blocks: %d\n", info->allocated_blocks);
	printf("Number of malloc calls: %d\n", info->malloc_calls);
	printf("Number of fragmentations: %d\n", info->fragmentations);
	printf("Number of free calls: %d\n", info->free_calls);
	for (int sz = 1; sz <= segregated_free_lists->size; sz++) {
		if (segregated_free_lists->v[sz]->size == 0) {
			continue;
		}
		printf("Blocks with %d bytes - %d free block(s) :", sz, segregated_free_lists->v[sz]->size);
		dll_node_t *node = segregated_free_lists->v[sz]->head;
		while (node) {
			printf(" 0x%x", get_address(node->data));
			node = node->nxt;
		}
		printf("\n");
	}
	printf("Allocated blocks :");
	dll_node_t *node = allocated_blocks->head;
	while (node) {
		printf(" (0x%x - %d)", get_address(node->data), get_size(node->data));
		node = node->nxt;
	}
	printf("\n");
	printf("-----DUMP-----\n");
}

void malloc_command(general_info_t *info, dll_array_t *segregated_free_lists, doubly_linked_list_t *allocated_blocks, int no_bytes)
{
	if (no_bytes <= segregated_free_lists->size && segregated_free_lists->v[no_bytes]->size != 0) {
		dll_node_t *old_node = segregated_free_lists->v[no_bytes]->head;
		dll_pop_front(segregated_free_lists->v[no_bytes]);
		char *v = (char *)malloc(no_bytes * sizeof(char));
		void *data = create_data(get_id(old_node->data), get_address(old_node->data), no_bytes, v);
		dll_node_t *new_node = create_node(data);
		dll_insert(allocated_blocks, new_node);
		delete_node(old_node);
		info->allocated_memory += no_bytes;
		info->free_memory -= no_bytes;
		info->allocated_blocks++;
		info->free_blocks--;
		info->malloc_calls++;
		return;
	}
	for (int sz = no_bytes + 1; sz <= segregated_free_lists->size; sz++) {
		if (segregated_free_lists->v[sz]->size == 0) {
			continue;
		}
		dll_node_t *front = segregated_free_lists->v[sz]->head;
		dll_pop_front(segregated_free_lists->v[sz]);
		char *v = (char *)malloc(no_bytes * sizeof(char));
		void *allocated_data = create_data(get_id(front->data), get_address(front->data), no_bytes, v);
		dll_node_t *allocated_node = create_node(allocated_data);
		void *free_data = create_data(get_id(front->data), get_address(front->data) + no_bytes, sz - no_bytes, 0);
		dll_node_t *free_node = create_node(free_data);
		dll_insert(allocated_blocks, allocated_node);
		dll_insert(segregated_free_lists->v[sz - no_bytes], free_node);
		delete_node(front);
		info->allocated_memory += no_bytes;
		info->free_memory -= no_bytes;
		info->allocated_blocks++;
		info->malloc_calls++;
		info->fragmentations++;
		return;
	}
	printf("Out of memory\n");
}

void free_command(general_info_t *info, dll_array_t *segregated_free_lists, doubly_linked_list_t *allocated_blocks, int address)
{
	dll_node_t *node = dll_lower_bound(allocated_blocks, address);
	if (node == NULL || get_address(node->data) != address) {
		printf("Invalid free\n");
		return;
	}
	dll_erase(allocated_blocks, node);
	int size = get_size(node->data);
	dll_insert(segregated_free_lists->v[size], node);
	info->allocated_memory -= size;
	info->free_memory += size;
	info->allocated_blocks--;
	info->free_blocks++;
	info->free_calls++;
}

int check_seg_fault(doubly_linked_list_t *allocated_blocks, int address, int no_bytes)
{
	dll_node_t *node = dll_lower_bound(allocated_blocks, address);
	if (node == NULL || get_address(node->data) + get_size(node->data) <= address) {
		return 1;
	}
	int true_size = get_address(node->data) + get_size(node->data) - address;
	int left = no_bytes - true_size;
	while (left > 0) {
		if (node->nxt == NULL || get_address(node->data) + get_size(node->data) != get_address(node->nxt->data)) {
			return 1;
		}
		node = node->nxt;
		left -= get_size(node->data);
	}
	return 0;
}

int read_command(doubly_linked_list_t *allocated_blocks, int address, int no_bytes)
{
	if (check_seg_fault(allocated_blocks, address, no_bytes)) {
		return 1;
	}
	dll_node_t *node = dll_lower_bound(allocated_blocks, address);
	int true_size = get_address(node->data) + get_size(node->data) - address;
	int left = no_bytes - true_size;
	int pos = 0;
	char *v = get_array(node->data);
	char *str = malloc((no_bytes + 1) * sizeof(char));
	memset(str, 0, (no_bytes + 1) * sizeof(char));
	for (int i = address - get_address(node->data); i < get_size(node->data) && pos < no_bytes; i++) {
		str[pos] = v[i];
		pos++;
	}
	while (left > 0) {
		node = node->nxt;
		v = get_array(node->data);
		for (int i = 0; i < get_size(node->data) && pos < no_bytes; i++) {
			str[pos] = v[i];
			pos++;
		}
		left -= get_size(node->data);
	}
	printf("%s\n", str);
	free(str);
	return 0;
}

int write_command(doubly_linked_list_t *allocated_blocks, int address, char str[MAX_STRING_LENGTH], int no_bytes)
{
	if (check_seg_fault(allocated_blocks, address, no_bytes)) {
		return 1;
	}
	dll_node_t *node = dll_lower_bound(allocated_blocks, address);
	int true_size = get_address(node->data) + get_size(node->data) - address;
	int left = no_bytes - true_size;
	int pos = 0;
	char *v = get_array(node->data);
	for (int i = address - get_address(node->data); i < get_size(node->data) && pos < no_bytes; i++) {
		v[i] = str[pos];
		pos++;
	}
	while (left > 0) {
		node = node->nxt;
		v = get_array(node->data);
		for (int i = 0; i < get_size(node->data) && pos < no_bytes; i++) {
			v[i] = str[pos];
			pos++;
		}
		left -= get_size(node->data);
	}
	return 0;
}

void destroy_heap(dll_array_t *segregated_free_lists, doubly_linked_list_t *allocated_blocks)
{
	for (int sz = 1; sz <= segregated_free_lists->size; sz++) {
		delete_list(segregated_free_lists->v[sz]);
	}
	free(segregated_free_lists->v);
	delete_list(allocated_blocks);
}