#ifndef _MID_LIST_H_
#define _MID_LIST_H_

#define FILE_NAME_MAXT_LEN 32

typedef struct LIST_Node_T {
	char filename[FILE_NAME_MAXT_LEN];
	struct LIST_Node_T *next;
} list_node_t;

/* Create a list header */
static list_node_t * create_list(void) {
	list_node_t * head;
	head = (list_node_t *) malloc(sizeof(list_node_t));
	if (NULL == head)
		return NULL;
	memset(head, 0, sizeof(list_node_t));
	head->next = NULL;
	return head;
}

/* Recursive destruction of the list */
static int destroy_list(list_node_t *node) {
	if (node->next == NULL)
		free(node);
	else {
		destroy_list(node->next);
		free(node);
	}
	return 0;
}

static int list_add(list_node_t *node, char *filename) {
	list_node_t *new_node;
	while (node->next) {
		node = node->next;
	}
	new_node = (list_node_t *) malloc(sizeof(list_node_t));
	if (NULL == new_node)
		return -1;
	memset(new_node, 0, sizeof(list_node_t));
	strcpy(new_node->filename, filename);
	node->next = new_node;
	return 0;
}

static int list_add_media(list_node_t *node, char *filename) {
	list_node_t *new_node;
	while (node->next) {
		node = node->next;
	}
	new_node = (list_node_t *) malloc(sizeof(list_node_t));
	if (NULL == new_node)
		return -1;
	memset(new_node, 0, sizeof(list_node_t));
	strncpy(new_node->filename, filename, FILE_NAME_MAXT_LEN);
	node->next = new_node;
	return 0;
}

static list_node_t *list_del(list_node_t *node) {
	list_node_t *del_node;
	if (node->next) {
		del_node = node->next;
		node->next = node->next->next;
		return del_node;
	} else {
		return NULL;
	}
}

static list_node_t *list_del_index(list_node_t *head, int index) {
	int cont = 0;
	list_node_t *tmp_node;
	list_node_t *tmp_node1;

	tmp_node1 = head;
	tmp_node = head->next;

	while (tmp_node) {
		cont++;

		if (index == cont) {
			tmp_node1->next = tmp_node->next;
			break;
		}
		tmp_node1 = tmp_node;
		tmp_node = tmp_node->next;
	}
	return tmp_node;
}

static int list_get_total(list_node_t *head) {
	int cont = 0;
	list_node_t *tmp_node;
	if (!head)
		return 0;
	tmp_node = head->next;

	while (tmp_node) {
		cont++;
		tmp_node = tmp_node->next;
	}
	return cont;
}

#endif
