#include "commands.h"

int parse_command(char command[MAX_STRING_LENGTH], general_info_t *info, dll_array_t *segregated_free_lists, doubly_linked_list_t *allocated_blocks)
{
	(void) segregated_free_lists;
	(void) allocated_blocks;
	if (!strcmp(command, "INIT_HEAP")) {
		int start_address, no_lists, list_size;
		scanf("%x %d %d %d\n", &start_address, &no_lists, &list_size, &info->reconstruction);
		printf("Comanda este INIT_HEAP, avand parametrii: %d %d %d %d\n", start_address, no_lists, list_size, info->reconstruction);
		initialise_heap(info, segregated_free_lists, start_address, no_lists, list_size);
	}

	if (!strcmp(command, "MALLOC")) {
		int no_bytes;
		scanf("%d\n", &no_bytes);
		printf("Comanda este MALLOC, avand parametrul %d\n", no_bytes);
		malloc_command(info, segregated_free_lists, allocated_blocks, no_bytes);
	}

	if (!strcmp(command, "FREE")) {
		int address;
		scanf("%x\n", &address);
		printf("Comanda este FREE, avand parametrul %d\n", address);
		if (!address) {
			return 0;
		}
		free_command(info, segregated_free_lists, allocated_blocks, address);
	}

	if (!strcmp(command, "READ")) {
		int address, no_bytes;
		scanf("%x %d\n", &address, &no_bytes);
		printf("Comanda este READ, avand parametrii: %d %d\n", address, no_bytes);
	}

	if (!strcmp(command, "WRITE")) {
		int address, no_bytes;
		char data[MAX_STRING_LENGTH];
		scanf("%x %s %d", &address, data, &no_bytes);
		printf("Comanda este WRITE, avand parametrii: %x %s %d\n", address, data, no_bytes);
	}

	if (!strcmp(command, "DUMP_MEMORY")) {
		printf("Comanda chiar este DUMP_MEMORY\n");
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
			void *data = create_data(id, address, sz);
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
	printf("Number of free blocks: %d\n", info->free_blocks);
	printf("Number of allocated blocks: %d\n", info->allocated_blocks);
	printf("Number of malloc calls: %d\n", info->malloc_calls);
	printf("Number of fragmentations: %d\n", info->fragmentations);
	printf("Number of free calls: %d\n", info->free_calls);
	for (int sz = 1; sz <= segregated_free_lists->size; sz++) {
		if (segregated_free_lists->v[sz]->size == 0) {
			continue;
		}
		printf("Blocks with %d bytes - %d free block(s) : ", sz, segregated_free_lists->v[sz]->size);
		dll_node_t *node = segregated_free_lists->v[sz]->head;
		while (node) {
			printf("0x%x ", get_address(node->data));
			node = node->nxt;
		}
		printf("\n");
	}
	printf("Allocated blocks : ");
	dll_node_t *node = allocated_blocks->head;
	while (node) {
		printf("(0x%x - %d) ", get_address(node->data), get_size(node->data));
		node = node->nxt;
	}
	printf("\n");
	printf("-----DUMP-----\n");
}

void malloc_command(general_info_t *info, dll_array_t *segregated_free_lists, doubly_linked_list_t *allocated_blocks, int no_bytes)
{
	if (segregated_free_lists->v[no_bytes]->size != 0) {
		dll_node_t *node = segregated_free_lists->v[no_bytes]->head;
		dll_pop_front(segregated_free_lists->v[no_bytes]);
		dll_insert(allocated_blocks, node);
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
		dll_node_t *front = segregated_free_lists->v[no_bytes]->head;
		dll_pop_front(segregated_free_lists->v[no_bytes]);
		void *allocated_data = create_data(get_id(front), get_address(front), no_bytes);
		dll_node_t *allocated_node = create_node(allocated_data);
		void *free_data = create_data(get_id(front), get_address(front) + no_bytes, sz - no_bytes);
		dll_node_t *free_node = create_node(free_data);
		dll_insert(allocated_blocks, allocated_node);
		dll_insert(segregated_free_lists->v[sz - no_bytes], free_node);
		delete_node(front);
		info->allocated_memory += no_bytes;
		info->free_memory -= no_bytes;
		info->allocated_blocks++;
		info->malloc_calls++;
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
	if (allocated_blocks->head == node) {
		allocated_blocks->head = node->nxt;
	}
	if (node->prv) {
		node->prv->nxt = node->nxt;
	}
	if (node->nxt) {
		node->nxt->prv = node->prv;
	}
	int size = get_size(node->data);
	dll_insert(segregated_free_lists->v[size], node);
	info->allocated_memory += size;
	info->free_memory -= size;
	info->allocated_blocks--;
	info->free_blocks++;
	info->free_calls++;
}

void destroy_heap(dll_array_t *segregated_free_lists, doubly_linked_list_t *allocated_blocks)
{
	free(segregated_free_lists->v);
}