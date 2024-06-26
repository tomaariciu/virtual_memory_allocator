#include "dll.h"

void *create_data(int id, int address, int size, char *v)
{
	memory_data_t *data = malloc(sizeof(memory_data_t));
	DIE(!data, "Memory allocation failed!\n");
	data->id = id;
	data->address = address;
	data->size = size;
	data->v = v;
	return (void *)data;
}

int get_id(void *data)
{
	return ((memory_data_t *)data)->id;
}

int get_address(void *data)
{
	return ((memory_data_t *)data)->address;
}

int get_size(void *data)
{
	return ((memory_data_t *)data)->size;
}

char *get_array(void *data)
{
	return ((memory_data_t *)data)->v;
}

int get_end_addr(void *data)
{
	return get_address(data) + get_size(data);
}

dll_node_t *create_node(void *data)
{
	dll_node_t *new_node = malloc(sizeof(dll_node_t));
	DIE(!new_node, "Memory allocation failed!\n");
	new_node->data = data;
	new_node->nxt = NULL;
	new_node->prv = NULL;
	return new_node;
}

void delete_node(dll_node_t *node)
{
	free(node->data);
	free(node);
}

doubly_linked_list_t *create_dll(void)
{
	doubly_linked_list_t *list = malloc(sizeof(doubly_linked_list_t));
	DIE(!list, "Memory allocation failed!\n");
	list->head = NULL;
	list->size = 0;
	return list;
}

void delete_list(doubly_linked_list_t *list)
{
	dll_node_t *curr_node = list->head;
	for (int i = 0; i < list->size; i++) {
		dll_node_t *nxt_node = curr_node->nxt;
		char *v = get_array(curr_node->data);
		free(v);
		delete_node(curr_node);
		curr_node = nxt_node;
	}
	free(list);
}

void dll_pop_front(doubly_linked_list_t *list)
{
	if (list->head->nxt)
		list->head->nxt->prv = NULL;
	list->head = list->head->nxt;
	list->size--;
}

dll_node_t *dll_lower_bound(doubly_linked_list_t *list, int address)
{
	if (!list->head || address < get_address(list->head->data))
		return NULL;
	dll_node_t *aux = list->head;
	while (aux->nxt && address >= get_address(aux->nxt->data))
		aux = aux->nxt;
	return aux;
}

void dll_insert(doubly_linked_list_t *list, dll_node_t *node)
{
	list->size++;
	dll_node_t *aux = dll_lower_bound(list, get_address(node->data));
	if (!aux) {
		node->nxt = list->head;
		if (list->head)
			list->head->prv = node;
		node->prv = NULL;
		list->head = node;
	} else {
		node->nxt = aux->nxt;
		node->prv = aux;
		if (aux->nxt)
			aux->nxt->prv = node;
		aux->nxt = node;
	}
}

void dll_erase(doubly_linked_list_t *list, dll_node_t *node)
{
	if (list->head == node)
		list->head = node->nxt;
	if (node->prv)
		node->prv->nxt = node->nxt;
	if (node->nxt)
		node->nxt->prv = node->prv;
	list->size--;
}
