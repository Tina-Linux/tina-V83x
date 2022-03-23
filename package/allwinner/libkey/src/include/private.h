#ifndef __FETCH_ENV_H__
#define __FETCH_ENV_H__

int sunxi_private_store_write(const char *item_name, char *buffer, int length);

int sunxi_private_store_read(const char *item_name, char *buffer,
			     int buffer_len, int *data_len);

int sunxi_private_store_erase(void);

int sunxi_private_store_list(void);

int private_bare_key_support(void);
#endif
