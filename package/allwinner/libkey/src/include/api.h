#ifndef __API_H__
#define __API_H__

/*
 *	function:	write the key data to secure storage or private partition
 *	input	:	item_name	: key name
 *				buffer	 	:		key data
 *				length		:		key lenth
 *	ruturn	:	0: seccure  -1: fail
 */
int key_data_write(const char *item_name, char *buffer, int length);

/*
 *	function:	get key data from secure storage or private partition
 *	input	:	item_name	: key name
 *				buffer	 	:
 *				buffer_len	:
 * 				data_len	: key lenth  (get key lenth from private partition or secure storage)
 *	ruturn	:	0: seccure  -1: fail
 */
int key_data_read(const char *item_name, char *buffer, int buffer_len,
		  int *data_len);

#endif
