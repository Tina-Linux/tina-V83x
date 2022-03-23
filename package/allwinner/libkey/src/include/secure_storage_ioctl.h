#ifndef __SECURE_STORAGE_IOCTRL_H__
#define __SECURE_STORAGE_IOCTRL_H__

#define SST_STORAGE_READ    _IO('V', 1)
#define SST_STORAGE_WRITE   _IO('V', 2)
#define OEM_PATH            ("/dev/sst_storage")
struct sst_storage_data {
	int id;
	unsigned char *buf;
	unsigned int len;
	unsigned int offset;
};
extern int _nand_read_ioctl(int id, unsigned char *buf, ssize_t len);
extern int _nand_write_ioctl(int id, unsigned char *buf, ssize_t len);
extern int _sd_read_ioctl(int id, unsigned char *buf, ssize_t len);
extern int _sd_read_backup_ioctl(int id, unsigned char *buf, ssize_t len);
extern int _sd_write_ioctl(int id, unsigned char *buf, ssize_t len);

#endif
