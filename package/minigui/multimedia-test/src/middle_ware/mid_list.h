#ifndef _MID_LIST_H_
#define _MID_LIST_H_

#define FILE_NAME_MAXT_LEN 32
#define FILE_PATH_MAXT_LEN 64

typedef enum MEDIA_FILE_TYPE_T {
	MEDIA_F_TYPE_VIDEO, MEDIA_F_TYPE_PICTURE, MEDIA_F_TYPE_
} media_file_type_t;

typedef struct LIST_Node_T {
	char path[FILE_PATH_MAXT_LEN];
	media_file_type_t type;
	struct LIST_Node_T *next;
	struct LIST_Node_T *pre;
} list_node_t;

typedef struct LIST_HEAD_T {
	int total;
	int cur_index;
	struct LIST_Node_T *current_node;
	struct LIST_Node_T *first_node;
} list_head_t;

/* Create a list header */
static list_head_t * create_list(void) {
	list_head_t * head;
	head = (list_head_t *) malloc(sizeof(list_head_t));
	if (NULL == head)
		return NULL;
	memset(head, 0, sizeof(list_head_t));
	return head;
}

/* Destroy the linked list */
static int destroy_list(list_head_t *head) {
	list_node_t *tmp_node;
	if (!head) {
		printf("list head is null!\n");
		return -1;
	}
	while (head->total > 0) {
		printf("total = %d\n", head->total);
		tmp_node = head->first_node;
		if (tmp_node->next == tmp_node) {
			head->first_node = NULL;
		} else {
			head->first_node = tmp_node->next;
			tmp_node->pre->next = tmp_node->next;
			tmp_node->next->pre = tmp_node->pre;
		}
		free(tmp_node);
		head->total--;
	}
	free(head);
	return 0;
}

static list_node_t *list_get_current_node(list_head_t *head) {
	if (!head) {
		printf("list head is null!\n");
		return NULL;
	}
	return head->current_node;
}

static int list_get_total_num(list_head_t *head) {
	if (!head) {
		printf("list head is null!\n");
		return 0;
	}
	return head->total;
}

static int list_get_cur_index(list_head_t *head) {
	if (!head) {
		printf("list head is null!\n");
		return 0;
	}
	return head->cur_index;
}

static list_node_t *list_get_next_node(list_head_t *head) {
	if (!head) {
		printf("list head is null!\n");
		return NULL;
	}
	if (!head->current_node) {
		return NULL;
	}
	head->current_node = head->current_node->next;
	if (head->current_node == head->first_node) {
		head->cur_index = 1;
	} else {
		head->cur_index++;
	}
	if (head->cur_index > head->total) {
		head->cur_index = 1;
	}
	return head->current_node;
}

static list_node_t *list_get_pre_node(list_head_t *head) {
	if (!head) {
		printf("list head is null!\n");
		return NULL;
	}
	if (!head->current_node) {
		return NULL;
	}
	head->current_node = head->current_node->pre;
	if (head->current_node == head->first_node) {
		head->cur_index = 1;
	} else {
		head->cur_index--;
	}
	if (head->cur_index < 1) {
		head->cur_index = head->total;
	}
	return head->current_node;
}

static void list_del_cur_node(list_head_t *head) {
	list_node_t *tmp_node;
	if (!head) {
		printf("list head is null!\n");
		return;
	}
	if (!head->current_node) {
		return;
	}
	if (head->first_node == head->current_node) {
		tmp_node = head->current_node;
		if (tmp_node->pre == tmp_node) {
			head->current_node = NULL;
			head->first_node = NULL;
			head->cur_index = 0;
		} else {
			head->current_node = tmp_node->next;
			head->first_node = tmp_node->next;
			tmp_node->pre->next = tmp_node->next;
			tmp_node->next->pre = tmp_node->pre;
		}
		free(tmp_node);
	} else {
		if (head->cur_index == head->total)
			head->cur_index = 1;
		tmp_node = head->current_node;
		head->current_node = tmp_node->next;
		tmp_node->pre->next = tmp_node->next;
		tmp_node->next->pre = tmp_node->pre;
		free(tmp_node);
	}
	head->total--;
}

static int list_append(list_head_t *head, char *path, media_file_type_t type) {
	list_node_t *new_node;
	list_node_t *tmp_node;
	if (!head) {
		printf("list head is null!\n");
		return -1;
	}
	if (!path) {
		printf("path is null!\n");
		return -1;
	}
	new_node = (list_node_t *) malloc(sizeof(list_node_t));
	if (!new_node) {
		printf("malloc failed!\n");
		return -1;
	}
	memset(new_node, 0, sizeof(list_node_t));
	strncpy(new_node->path, path, FILE_PATH_MAXT_LEN);
	new_node->type = type;
	if (!head->first_node) {   /* Add the first node */
		head->first_node = new_node;
		new_node->pre = new_node;
		new_node->next = new_node;
		head->current_node = head->first_node;
		head->cur_index = 1;
	} else {
		tmp_node = head->first_node;
		while (tmp_node->next != head->first_node) {
			tmp_node = tmp_node->next;
		}
		tmp_node->next = new_node;
		new_node->pre = tmp_node;
		new_node->next = head->first_node;
		head->first_node->pre = new_node;
	}
	head->total++;
	return 0;
}

#endif
