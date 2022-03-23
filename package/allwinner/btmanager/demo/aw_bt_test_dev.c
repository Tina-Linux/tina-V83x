#include <string.h>
#include <stdio.h>
#include "aw_bt_test_dev.h"


int dev_add_device(dev_list_t *dev_list,const char *name,const char *addr)
{
	dev_node_t *dev_node = NULL;
	dev_node_t *dev_tail = NULL;

	if (!dev_list) {
		printf("%s: dev_list is NULL\n", __func__);
		return -1;
	}

	if (name == NULL || addr == NULL) {
		printf("device's name or addr is NULL, add device failed!\n");
		return -1;
	}

	dev_node = (dev_node_t *)calloc(1, sizeof(dev_node_t));

	if (dev_node == NULL) {
		printf("%s: calloc for dev_node failed\n", __func__);
		return -1;
	}

	strncpy(dev_node->dev_name, name, MAX_BT_NAME_LEN + 1);
	dev_node->dev_name[MAX_BT_NAME_LEN] = '\0';

	strncpy(dev_node->dev_addr, addr, MAX_BT_ADDR_LEN + 1);
	dev_node->dev_addr[MAX_BT_ADDR_LEN] = '\0';

	dev_node->next = NULL;

	pthread_mutex_lock(&dev_list->lock);

	if (dev_list->list_cleared) {
		printf("dev_list is cleared, nothing could be done\n");
		free(dev_node);
		pthread_mutex_unlock(&dev_list->lock);
		return -1;
	}

	if (dev_list->head == NULL) {
		dev_list->head = dev_node;
		dev_list->tail = dev_node;
		dev_node->front = NULL;
	} else {
		dev_tail = dev_list->tail;
		dev_node->front = dev_tail;
		dev_tail->next = dev_node;
		dev_list->tail = dev_node;
	}
	pthread_mutex_unlock(&dev_list->lock);
	return 0;
}

dev_node_t *dev_find_device(dev_list_t *dev_list,const char *addr)
{
	dev_node_t *dev_node = NULL;

	if (!dev_list) {
		printf("%s: dev_list is NULL\n", __func__);
		return NULL;
	}

	pthread_mutex_lock(&dev_list->lock);

	if (dev_list->list_cleared) {
		printf("dev_list is cleared, nothing could be done\n");
		pthread_mutex_unlock(&dev_list->lock);
		return NULL;
	}

	dev_node = dev_list->head;
	if (dev_node == NULL) {
		printf("%s: device list is NULL\n", __func__);
		pthread_mutex_unlock(&dev_list->lock);
		return NULL;
	}

	while (dev_node != NULL) {
		if (!strcmp(dev_node->dev_addr, addr)) {
			pthread_mutex_unlock(&dev_list->lock);
			return dev_node;
		}
		else
			dev_node = dev_node->next;
	}
	pthread_mutex_unlock(&dev_list->lock);

	return NULL;
}

static dev_node_t *dev_find_device_inner(dev_list_t *dev_list,const char *addr)
{
	dev_node_t *dev_node = NULL;

	if (!dev_list) {
		printf("%s: dev_list is NULL\n", __func__);
		return NULL;
	}

	dev_node = dev_list->head;
	if (dev_node == NULL) {
		printf("%s: device node is NULL\n", __func__);
		return NULL;
	}

	while (dev_node != NULL) {
		if (!strcmp(dev_node->dev_addr, addr))
			return dev_node;
		else
			dev_node = dev_node->next;
	}

	return NULL;
}

bool dev_remove_device(dev_list_t *dev_list,const char *addr)
{
	dev_node_t *dev_node = NULL;

	if (!dev_list) {
		printf("%s: dev_list is NULL\n", __func__);
		return false;
	}


	pthread_mutex_lock(&dev_list->lock);

	if (dev_list->list_cleared) {
		printf("dev_list is cleared, nothing could be done\n");
		pthread_mutex_unlock(&dev_list->lock);
		return false;
	}

	dev_node = dev_find_device_inner(dev_list, addr);

	if (dev_node == NULL) {
		printf("device is not found in device list\n");
		pthread_mutex_unlock(&dev_list->lock);
		return true;
	}

	if (dev_node == dev_list->head) {
		if (dev_node->next == NULL) { //dev_node is the only node in dev_list
			free(dev_node);
			dev_list->head = NULL;
			dev_list->tail = NULL;
		} else {
			dev_list->head = dev_node->next;
			dev_node->next->front = NULL;
			free(dev_node);
		}
	} else {
		dev_node->front->next = dev_node->next;
		if (dev_node->next) { //dev_node is not the tail of dev_list
			dev_node->next->front = dev_node->front;
		} else {
			dev_list->tail = dev_node->front;
		}
		free(dev_node);
	}
	pthread_mutex_unlock(&dev_list->lock);

	return true;
}


dev_list_t *dev_list_new()
{
	dev_list_t *list = NULL;

	list = (dev_list_t *)calloc(1, sizeof(dev_list_t));
	if (list != NULL) {
		list->list_cleared = false;
		if ( 0 != pthread_mutex_init(&list->lock, NULL)) {
			printf("init mutex for dev list failed\n");
			free(list);
			return NULL;
		}
		return list;
	}
}

void dev_list_clear(dev_list_t *list)
{
	dev_node_t *dev_node = NULL;
	dev_node_t *dev_node_free = NULL;

	pthread_mutex_lock(&list->lock);
/*indicate the list is to be free, so the other operaiton can not be
*done anymore between return dev_list_clear() and pthread_mutex_destroy(&list->lock)
*/
	list->list_cleared = true;
	dev_node = list->head;

	while (dev_node != NULL) {
		dev_node_free = dev_node;
		dev_node = dev_node->next;
		free(dev_node_free);
	}
	pthread_mutex_unlock(&list->lock);
}

void dev_list_free(dev_list_t *list)
{
	if (list != NULL) {
		dev_list_clear(list);
		pthread_mutex_destroy(&list->lock);
		free(list);
		return;
	}
}
