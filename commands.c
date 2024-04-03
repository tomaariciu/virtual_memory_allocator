#include "commands.h"

int parse_command(char command[MAX_STRING_LENGTH], general_info_t *info,
				  dll_array_t *sfl, doubly_linked_list_t *alloc_list)
{
	if (!strcmp(command, "INIT_HEAP")) {
		int start_address, no_lists, list_size;
		scanf("%x %d %d %d\n", &start_address, &no_lists, &list_size,
			  &info->reconstruction);
		initialise_heap(info, sfl, start_address, no_lists, list_size);
	}

	if (!strcmp(command, "MALLOC")) {
		int no_bytes;
		scanf("%d\n", &no_bytes);
		malloc_command(info, sfl, alloc_list, no_bytes);
	}

	if (!strcmp(command, "FREE")) {
		int address;
		scanf("%x\n", &address);
		if (!address)
			return 0;
		free_command(info, sfl, alloc_list, address);
	}

	if (!strcmp(command, "READ")) {
		int address, no_bytes;
		scanf("%x %d\n", &address, &no_bytes);
		int seg_fault = read_command(alloc_list, address, no_bytes);
		if (seg_fault) {
			printf("Segmentation fault (core dumped)\n");
			dump_memory(info, sfl, alloc_list);
			destroy_heap(sfl, alloc_list);
			return 1;
		}
	}

	if (!strcmp(command, "WRITE")) {
		int address, no_bytes;
		char data[MAX_STRING_LENGTH], aux[MAX_STRING_LENGTH];
		memset(data, 0, MAX_STRING_LENGTH);
		memset(aux, 0, MAX_STRING_LENGTH);
		scanf("%x \"", &address);
		fgets(data, MAX_STRING_LENGTH, stdin);
		int last_quotes = 0, len = (int)strlen(data);
		for (int i = 0; i < len; i++) {
			if (data[i] == '"')
				last_quotes = i;
		}
		data[last_quotes] = 0;
		for (int i = last_quotes + 1; i < len; i++) {
			aux[i - last_quotes - 1] = data[i];
			data[i] = 0;
		}
		sscanf(aux, "%d", &no_bytes);
		if ((int)strlen(data) < no_bytes)
			no_bytes = strlen(data);
		int seg_fault = write_command(alloc_list, address, data, no_bytes);
		if (seg_fault) {
			printf("Segmentation fault (core dumped)\n");
			dump_memory(info, sfl, alloc_list);
			destroy_heap(sfl, alloc_list);
			return 1;
		}
	}

	if (!strcmp(command, "DUMP_MEMORY"))
		dump_memory(info, sfl, alloc_list);

	if (!strcmp(command, "DESTROY_HEAP")) {
		destroy_heap(sfl, alloc_list);
		return 1;
	}

	return 0;
}

void initialise_heap(general_info_t *info, dll_array_t *sfl, int start_address,
					 int no_lists, int list_size)
{
	info->total_memory = no_lists * list_size;
	int max_size = (8 << (no_lists - 1));
	sfl->size = max_size;
	sfl->v = (doubly_linked_list_t **)
			 malloc((max_size + 1) * sizeof(doubly_linked_list_t *));
	DIE(!sfl, "Memory allocation failed!\n");

	for (int i = 1; i <= max_size; i++)
		sfl->v[i] = create_dll();

	int id = 0, address = start_address;
	for (int sz = 8; sz <= max_size; sz <<= 1) {
		dll_node_t *last_node = NULL;
		for (int i = 0; i < list_size / sz; i++) {
			void *data = create_data(id, address, sz, 0);
			dll_node_t *new_node = create_node(data);
			if (!sfl->v[sz]->head)
				sfl->v[sz]->head = new_node;
			if (last_node)
				last_node->nxt = new_node;
			new_node->prv = last_node;
			sfl->v[sz]->size++;
			id++;
			address += sz;
			info->free_memory += sz;
			info->free_blocks++;
			last_node = new_node;
		}
	}
}

void dump_memory(general_info_t *info, dll_array_t *sfl,
				 doubly_linked_list_t *alloc_list)
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
	for (int sz = 1; sz <= sfl->size; sz++) {
		if (sfl->v[sz]->size == 0)
			continue;
		printf("Blocks with %d bytes - %d free block(s) :", sz,
			   sfl->v[sz]->size);
		dll_node_t *node = sfl->v[sz]->head;
		while (node) {
			printf(" 0x%x", get_address(node->data));
			node = node->nxt;
		}
		printf("\n");
	}
	printf("Allocated blocks :");
	dll_node_t *node = alloc_list->head;
	while (node) {
		printf(" (0x%x - %d)", get_address(node->data), get_size(node->data));
		node = node->nxt;
	}
	printf("\n");
	printf("-----DUMP-----\n");
}

void malloc_command(general_info_t *info, dll_array_t *sfl,
					doubly_linked_list_t *alloc_list, int no_bytes)
{
	if (no_bytes <= sfl->size && sfl->v[no_bytes]->size != 0) {
		dll_node_t *old_node = sfl->v[no_bytes]->head;
		dll_pop_front(sfl->v[no_bytes]);
		char *v = (char *)malloc(no_bytes * sizeof(char));
		int id = get_id(old_node->data), addr = get_address(old_node->data);
		void *data = create_data(id, addr, no_bytes, v);
		dll_node_t *new_node = create_node(data);
		dll_insert(alloc_list, new_node);
		delete_node(old_node);
		info->allocated_memory += no_bytes;
		info->free_memory -= no_bytes;
		info->allocated_blocks++;
		info->free_blocks--;
		info->malloc_calls++;
		return;
	}
	for (int sz = no_bytes + 1; sz <= sfl->size; sz++) {
		if (sfl->v[sz]->size == 0)
			continue;
		dll_node_t *front = sfl->v[sz]->head;
		dll_pop_front(sfl->v[sz]);
		char *v = (char *)malloc(no_bytes * sizeof(char));
		int id = get_id(front->data), addr = get_address(front->data);
		void *allocated_data = create_data(id, addr, no_bytes, v);
		dll_node_t *allocated_node = create_node(allocated_data);
		void *free_data = create_data(id, addr + no_bytes, sz - no_bytes, 0);
		dll_node_t *free_node = create_node(free_data);
		dll_insert(alloc_list, allocated_node);
		dll_insert(sfl->v[sz - no_bytes], free_node);
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

dll_node_t *find_node(dll_array_t *sfl, int address)
{
	int best_address = 0;
	dll_node_t *node = NULL;
	for (int sz = 1; sz <= sfl->size; sz++) {
		if (sfl->v[sz]->size == 0)
			continue;
		dll_node_t *aux = dll_lower_bound(sfl->v[sz], address);
		if (aux && get_address(aux->data) > best_address) {
			best_address = get_address(aux->data);
			node = aux;
		}
	}
	return node;
}

void free_command(general_info_t *info, dll_array_t *sfl,
				  doubly_linked_list_t *alloc_list, int address)
{
	dll_node_t *node = dll_lower_bound(alloc_list, address);
	if (!node || get_address(node->data) != address) {
		printf("Invalid free\n");
		return;
	}
	dll_erase(alloc_list, node);
	char *v = get_array(node->data);
	free(v);
	int size = get_size(node->data);
	info->allocated_memory -= size;
	info->free_memory += size;
	info->free_calls++;
	info->allocated_blocks--;
	if (info->reconstruction == 0) {
		void *new_data = create_data(get_id(node->data), address, size, 0);
		delete_node(node);
		dll_node_t *new_node = create_node(new_data);
		dll_insert(sfl->v[size], new_node);
		info->free_blocks++;
		return;
	}
	if (info->reconstruction == 1) {
		int new_address = address, new_size = size, id = get_id(node->data);

		dll_node_t *left_node = find_node(sfl, address);
		if (left_node && get_end_addr(left_node->data) == address &&
			get_id(left_node->data) == id) {
			dll_erase(sfl->v[get_size(left_node->data)], left_node);
			new_address = get_address(left_node->data);
			new_size += get_size(left_node->data);
			delete_node(left_node);
			info->free_blocks--;
		}

		dll_node_t *right_node = find_node(sfl, address + size);
		if (right_node && address + size == get_address(right_node->data) &&
			get_id(right_node->data) == id) {
			dll_erase(sfl->v[get_size(right_node->data)], right_node);
			new_size += get_size(right_node->data);
			delete_node(right_node);
			info->free_blocks--;
		}
		void *new_data = create_data(id, new_address, new_size, 0);
		delete_node(node);
		dll_node_t *new_node = create_node(new_data);
		dll_insert(sfl->v[new_size], new_node);
		info->free_blocks++;
	}
}

int check_seg_fault(doubly_linked_list_t *alloc_list, int address, int no_bytes)
{
	dll_node_t *node = dll_lower_bound(alloc_list, address);
	if (!node || get_end_addr(node->data) <= address)
		return 1;
	int true_size = get_end_addr(node->data) - address;
	int left = no_bytes - true_size;
	while (left > 0) {
		if (!node->nxt || get_end_addr(node->data) !=
			get_address(node->nxt->data))
			return 1;
		node = node->nxt;
		left -= get_size(node->data);
	}
	return 0;
}

int read_command(doubly_linked_list_t *alloc_list, int address, int no_bytes)
{
	if (check_seg_fault(alloc_list, address, no_bytes))
		return 1;

	dll_node_t *node = dll_lower_bound(alloc_list, address);
	int true_size = get_end_addr(node->data) - address;
	int left = no_bytes - true_size;
	int pos = 0;
	char *v = get_array(node->data);
	char *str = malloc((no_bytes + 1) * sizeof(char));
	memset(str, 0, (no_bytes + 1) * sizeof(char));
	int start_addr = address - get_address(node->data);
	for (int i = start_addr; i < get_size(node->data) && pos < no_bytes; i++) {
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

int write_command(doubly_linked_list_t *alloc_list, int address,
				  char str[MAX_STRING_LENGTH], int no_bytes)
{
	if (check_seg_fault(alloc_list, address, no_bytes))
		return 1;

	dll_node_t *node = dll_lower_bound(alloc_list, address);
	int true_size = get_end_addr(node->data) - address;
	int left = no_bytes - true_size;
	int pos = 0;
	char *v = get_array(node->data);
	int start_addr = address - get_address(node->data);
	for (int i = start_addr; i < get_size(node->data) && pos < no_bytes; i++) {
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

void destroy_heap(dll_array_t *sfl, doubly_linked_list_t *alloc_list)
{
	for (int sz = 1; sz <= sfl->size; sz++)
		delete_list(sfl->v[sz]);
	free(sfl->v);
	delete_list(alloc_list);
}
