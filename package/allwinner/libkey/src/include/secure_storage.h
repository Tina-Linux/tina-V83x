#ifndef __SECURE_STORAGE_H__
#define __SECURE_STORAGE_H__

int sunxi_secure_storage_init(void);
int sunxi_secure_storage_exit(int mode);
int sunxi_secure_object_read(const char *item_name, char *buffer,
			     int buffer_len, int *data_len);
int sunxi_secure_object_write(const char *item_name, char *buffer, int length);
int secure_storage_support(void);
int sunxi_secure_object_erase(void);
int sunxi_secure_object_list(void);

#endif
