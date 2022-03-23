#ifndef __COMMON_H__
#define __COMMON_H__

/*
 *  commond parameters
 */
#define FLASH_TYPE_NAND     0
#define FLASH_TYPE_SD1      1
#define FLASH_TYPE_SD2      2
#define FLASH_TYPE_NOR		3
#define FLASH_TYPE_EMMC3	4
#define FLASH_TYPE_SPI_NAND	5
#define FLASH_TYPE_UNKNOW   -1

extern int flash_type_init(void);
extern int get_flash_type(void);

#endif /*end of define __COMMON_H__ */
