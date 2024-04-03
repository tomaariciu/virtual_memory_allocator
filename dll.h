#pragma once
#include "utils.h"
#include <stdlib.h>

typedef struct memory_data_t {
	int id, address, size;
	char *v;
} memory_data_t;

typedef struct dll_node_t {
	void *data;
	struct dll_node_t *prv, *nxt;
} dll_node_t;

typedef struct doubly_linked_list_t {
	dll_node_t *head;
	int size;
} doubly_linked_list_t;

typedef struct dll_array_t {
	doubly_linked_list_t **v;
	int size;
} dll_array_t;

void *create_data(int id, int address, int size);
int get_id(void *data);
int get_address(void *data);
int get_size(void *data);
char *get_array(void *data);
dll_node_t *create_node(void *data);
void delete_node(dll_node_t *node);
doubly_linked_list_t *create_dll();
void dll_pop_front(doubly_linked_list_t *list);
void dll_insert(doubly_linked_list_t *list, dll_node_t *node);
dll_node_t *dll_lower_bound(doubly_linked_list_t *list, int address);