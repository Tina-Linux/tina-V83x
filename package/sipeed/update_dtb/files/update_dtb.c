#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h> 

#define STAMP_VALUE                         (0x5F0A6C39)

#define DEV_BLOCK_SZIE                      (512)
#define TOC_START_BLOCK_ADDR                (32800) /* * 512 */

#define  TOC0_MAGIC                         ("TOC0.GLH")
#define  TOC_MAIN_INFO_MAGIC                (0x89119800)

#define ITEM_PARAMETER_NAME                 ("parameter")
#define ITEM_OPTEE_NAME			            ("optee")
#define ITEM_SCP_NAME			            ("scp")
#define ITEM_MONITOR_NAME		            ("monitor")
#define ITEM_UBOOT_NAME			            ("u-boot")
#define ITEM_LOGO_NAME			            ("logo")
#define ITEM_DTB_NAME			            ("dtb")
#define ITEM_SOCCFG_NAME		            ("soc-cfg")
#define ITEM_BDCFG_NAME			            ("board-cfg")
#define ITEM_ESM_IMG_NAME                   ("esm-img")
#define ITEM_SHUTDOWNCHARGE_LOGO_NAME       ("shutdowncharge")
#define ITEM_ANDROIDCHARGE_LOGO_NAME        ("androidcharge")
#define ITEM_EMMC_FW_NAME                   ("emmc-fw")
#define ITEM_NAME_SBROMSW_KEY		        (0x010303)

//增加安全启动下，toc1的头部数据结构
typedef struct sbrom_toc1_head_info
{
	char        name[16];       //user can modify
	uint32_t    magic;          //must equal TOC_uint32_t_MAGIC
	uint32_t    add_sum;
	uint32_t    serial_num;     //user can modify
	uint32_t    status;         //user can modify,such as TOC_MAIN_INFO_STATUS_ENCRYP_NOT_USED
	uint32_t    items_nr;       //total entry number
	uint32_t    valid_len;
	uint32_t    version_main;   //only one byte
	uint32_t    version_sub;    //two bytes
	uint32_t    reserved[3];    //reserved for future
	uint32_t    end;
} sbrom_toc1_head_info_t;

typedef struct sbrom_toc1_item_info
{
	char name[64];              //such as ITEM_NAME_SBROMSW_CERTIF
	uint32_t  data_offset;
	uint32_t  data_len;
	uint32_t  encrypt;          //0: no aes   //1: aes
	uint32_t  type;             //0: normal file, dont care  1: key certif  2: sign certif 3: bin file
	uint32_t  run_addr;         //if it is a bin file, then run on this address; if not, it should be 0
	uint32_t  index;            //if it is a bin file, this value shows the index to run; if not
	                            //if it is a certif file, it should equal to the bin file index
	                            //that they are in the same group
	                            //it should be 0 when it anyother data type
	uint32_t  reserved[69];     //reserved for future;
	uint32_t  end;
}sbrom_toc1_item_info_t;

uint32_t toc_start_blk_offset = TOC_START_BLOCK_ADDR;

static void usage(char *exec)
{
    printf("usage:\n\t%s /dev/sdc sipeed.dtb\n", exec);
}

uint sunxi_generate_checksum(void *buffer, uint length, uint src_sum)
{
	uint *buf;
	uint count;
	uint sum;

	count = length >> 2;
	sum   = 0;
	buf   = (uint32_t *)buffer;
	do {
		sum += *buf++;
		sum += *buf++;
		sum += *buf++;
		sum += *buf++;
	} while ((count -= 4) > (4 - 1));

	while (count-- > 0)
		sum += *buf++;

	sum = sum - src_sum + STAMP_VALUE;

	return sum;
}

int sunxi_verify_checksum(void *buffer, uint length, uint src_sum)
{
	uint sum;
	sum = sunxi_generate_checksum(buffer, length, src_sum);

	// printf("src sum=%x, check sum=%x\n", src_sum, sum);
	if (sum == src_sum)
		return 0;
	else
		return -1;
}

int mmc_read(char **buf, int *buf_len, char *blk_name, int blk_offset, int blk_count)
{
    char cmd[256];
    FILE *fp = NULL;
    struct stat sb;

    /* 从磁盘读取到文件 */
    snprintf(cmd, sizeof(cmd), "dd if=%s of=/tmp/read.bin bs=512 skip=%d count=%d > /dev/null 2>&1", blk_name, blk_offset, blk_count);
    // printf("read cmd: %s\n", cmd);

    if(system(cmd) != 0) {
        fprintf(stderr, "exec %s fail\n", cmd);
        *buf_len = 0;
        return -1;
    }

    /* 读取文件信息 */
    if(stat("/tmp/read.bin", &sb)) {
        fprintf(stderr, "open /tmp/read.bin fail\n");
        *buf_len = 0;
        return -1;
    }

    /* 判断文件大小 */
    if(sb.st_size > *buf_len) {
        fprintf(stderr, "/tmp/read.bin too big\n");
        *buf_len = 0;
        return -1;
    }
    *buf_len = sb.st_size;

    /* 读取文件 */
    fp = fopen("/tmp/read.bin", "rb");
    if(NULL == fp) {
        fprintf(stderr, "open /tmp/read.bin fail\n");
        *buf_len = 0;
        return -1;
    }
    fread(*buf, *buf_len, 1, fp);
    fclose(fp);
    
    return 0;
}

int mmc_write(char *buf, int buf_len, char *blk_name, int blk_offset)
{
    char cmd[256];
    FILE *fp = NULL;

    if(buf_len % 512) {
        fprintf(stderr, "write buf len err\n");
        return -1;
    }

    /* 写入文件 */
    fp = fopen("/tmp/write.bin", "wb+");
    if(NULL == fp) {
        fprintf(stderr, "write /tmp/write.bin fail\n");
        return -1;
    }
    fwrite(buf, buf_len, 1, fp);
    fclose(fp);

    /* 写入到磁盘 */
    snprintf(cmd, sizeof(cmd), "dd if=/tmp/write.bin of=%s bs=512 seek=%d count=%d conv=sync > /dev/null 2>&1", blk_name, blk_offset, (buf_len / 512));
    // printf("write cmd: %s\n", cmd);

    if(system(cmd) != 0) {
        fprintf(stderr, "exec %s fail\n", cmd);
        return -1;
    }

    return 0;
}

static void dump_package_head(sbrom_toc1_head_info_t *hdr)
{
    return ;
    printf("toc head:");
	printf("\tname: %s\n", hdr->name);
	printf("\tmagic: %x\n", hdr->magic);
	printf("\tadd_sum: %x\n", hdr->add_sum);
	printf("\tserial_num: %d\n", hdr->serial_num);
	printf("\tstatus: %d\n", hdr->status);
	printf("\titems_nr: %d\n", hdr->items_nr);
	printf("\tvalid_len: %d\n", hdr->valid_len);
	printf("\tversion_main: %d\n", hdr->version_main);
	printf("\tversion_sub: %d\n", hdr->version_sub);
	printf("\treserved[0]: %d\n", hdr->reserved[0]);
    printf("\treserved[1]: %d\n", hdr->reserved[1]);
    printf("\treserved[2]: %d\n", hdr->reserved[2]);
	printf("\tend: %d\n", hdr->end);
}

int read_boot_package(char *dev, char *package_buf, int buf_len)
{
    int stat = -1;
    int len = buf_len;
    sbrom_toc1_head_info_t *hdr = NULL;

    if(len % 512) {
        fprintf(stderr, "buf len err\n");
        return -1;
    }

    stat = mmc_read(&package_buf, &len, dev, toc_start_blk_offset, (len / 512));
    if((stat != 0) || (len <= 0)) {
        fprintf(stderr, "read boot package err\n");
        return -1;
    }

    hdr = (sbrom_toc1_head_info_t*)package_buf;
	if (hdr->magic != TOC_MAIN_INFO_MAGIC) {
		fprintf(stderr, "toc1 magic error\n");
		return -1;
	}

    if(hdr->valid_len > len) {
		fprintf(stderr, "hdr too large\n");
		return -1;
    }

    dump_package_head(hdr);

    return 0;
}

/* 获取到dtb的信息(偏移，大小) */
int get_dtb_info(char *buf, uint32_t *dtb_blk_addr, uint32_t *dtb_blk_size)
{
    int i = 0;
    sbrom_toc1_head_info_t *hdr = NULL;
    sbrom_toc1_item_info_t *item = NULL;
    sbrom_toc1_item_info_t *item_tmp = NULL;

    hdr = (sbrom_toc1_head_info_t*)buf;
    item = (sbrom_toc1_item_info_t*)(buf + sizeof(sbrom_toc1_head_info_t));
    item_tmp = item;

    int find_flag = 0;

	for (i = 0; i < hdr->items_nr; i++, item_tmp++) {
		if (strncmp(item_tmp->name, ITEM_DTB_NAME, sizeof(ITEM_DTB_NAME)) == 0) {
			find_flag = 1;
			break;
		}
	}

    if(!find_flag) {
        fprintf(stderr,"hdr can not found dtb\n");
        return -1;
    }

    uint32_t _dtb_addr = item_tmp->data_offset;
    uint32_t _dtb_len = item_tmp->data_len;

    if((_dtb_addr % DEV_BLOCK_SZIE) != 0) {
        fprintf(stderr,"dtb addr not align block size\n");
        return -1;
    }

    if((_dtb_len % DEV_BLOCK_SZIE) != 0) {
        fprintf(stderr,"dtb len not align block size\n");
        return -1;
    }

    *dtb_blk_addr = ((_dtb_addr / 512) + toc_start_blk_offset);
    *dtb_blk_size = (_dtb_len / 512);

    return 0;
}

/* 
    1. 从磁盘dd得到头部信息
    2. 获取到dtb的信息(偏移，大小)
    3. 备份旧的dtb(可选)
    4. dd新的dtb到磁盘
    5. dd更新头部的信息(新的dtb会更新大小)
 */
int main(int argc, char **argv)
{
    int i = 0, dump = 0;
    char cmd[1024];
    char dtb_path[1024];
    struct stat sb;

    char *toc_buf = NULL;
    int toc_buf_len = 1 * 1024 * 1024; /* 基本不会超过1M */

    /* 检查参数 */
    if(argc < 3) {
        usage(argv[0]);
        return -1;
    }

    if(argc >= 4) {
        if(strcmp(argv[3], "dump") == 0) {
            printf("dump dtb\n");
            dump = 1;
        }
    }

    if(strncmp("/dev/", argv[1], strlen("/dev/")) != 0) {
        fprintf(stderr, "error target %s\n", argv[1]);
        return -1;
    }

    if(strncmp("/dev/mtd", argv[1], strlen("/dev/mtd")) == 0) {
        toc_start_blk_offset = 128;
    }

    if(!dump && (stat(argv[2], &sb))) {
        fprintf(stderr, "open %s fail\n", argv[2]);
        return -1;
    }

    if(!dump && (sb.st_size % 512)) {
        fprintf(stderr, "%s size not align 512 Byte\n", argv[2]);
        return -1;
    }

    /* 申请内存 */
    toc_buf = (char*)malloc(toc_buf_len);
    if(NULL == toc_buf) {
        fprintf(stderr, "malloc fail %d\n", __LINE__);
        return -1;
    }

    ///////////////////////////////////////////////////////////////////////////
    /* 从磁盘dd得到头部信息 */
    if(read_boot_package(argv[1], toc_buf, toc_buf_len) != 0) {
        fprintf(stderr, "read boot package fail %d\n", __LINE__);
        if(toc_buf) {
            free(toc_buf);
            toc_buf = NULL;
        }
        return -1;
    }
    ///////////////////////////////////////////////////////////////////////////
    /* 获取dtb的偏移及大小 */
    uint32_t dtb_blk_offset = 0, dtb_blk_size = 0;
    uint32_t old_dtb_size = 0, new_dtb_size = 0;

    if(get_dtb_info(toc_buf, &dtb_blk_offset, &dtb_blk_size) != 0) {
        fprintf(stderr, "get dtb info fail %d\n", __LINE__);
        if(toc_buf) {
            free(toc_buf);
            toc_buf = NULL;
        }
        return -1;
    }
    old_dtb_size = dtb_blk_size * 512;

    ///////////////////////////////////////////////////////////////////////////
    /* 备份旧的dtb(可选) */
    sprintf(cmd, "dd if=%s of=/tmp/olddtb.bin bs=512 skip=%d count=%d conv=sync > /dev/null 2>&1", argv[1], dtb_blk_offset, dtb_blk_size);
    if(0x00 != system(cmd)) {
        fprintf(stderr, "exec %s fail\n", cmd);
        if(toc_buf) {
            free(toc_buf);
            toc_buf = NULL;
        }
        return -1;
    }
    if(dump) {
        printf("dump dtb over.\n");
        if(toc_buf) {
            free(toc_buf);
            toc_buf = NULL;
        }
        return 0;
    }
    // ///////////////////////////////////////////////////////////////////////////
    /* dd新的dtb到磁盘 */
    int dtb_file_sz = sb.st_size;
    realpath(argv[2], dtb_path);
    snprintf(cmd, sizeof(cmd), "dd if=%s of=%s bs=512 seek=%d count=%d conv=sync > /dev/null 2>&1", dtb_path, argv[1], dtb_blk_offset, (dtb_file_sz / 512));
    if(0x00 != system(cmd)) {
        fprintf(stderr, "exec %s fail\n", cmd);
        if(toc_buf) {
            free(toc_buf);
            toc_buf = NULL;
        }
        return -1;
    }
    ///////////////////////////////////////////////////////////////////////////
    /* 重新读取package */
    if(read_boot_package(argv[1], toc_buf, toc_buf_len) != 0) {
        fprintf(stderr, "read boot package fail %d\n", __LINE__);
        if(toc_buf) {
            free(toc_buf);
            toc_buf = NULL;
        }
        return -1;
    }

    if(get_dtb_info(toc_buf, &dtb_blk_offset, &dtb_blk_size) != 0) {
        fprintf(stderr, "get dtb info fail %d\n", __LINE__);
        if(toc_buf) {
            free(toc_buf);
            toc_buf = NULL;
        }
        return -1;
    }
    new_dtb_size = dtb_blk_size * 512;

    if(new_dtb_size != old_dtb_size) {
        printf("%d %d\n", old_dtb_size, new_dtb_size);
        printf("warning: two dtb size not equal\n");
        printf("TODO: update hdr->valid_len\n");
    }
    sbrom_toc1_head_info_t *hdr = (sbrom_toc1_head_info_t*)toc_buf;

    /* 需要更新校验 */
    hdr->add_sum = sunxi_generate_checksum((void*)toc_buf, hdr->valid_len, hdr->add_sum);
    int verify = sunxi_verify_checksum((void*)toc_buf, hdr->valid_len, hdr->add_sum);
    if(verify != 0) {
        fprintf(stderr, "verify fail %d\n", __LINE__);
        if(toc_buf) {
            free(toc_buf);
            toc_buf = NULL;
        }
        return -1;
    }

    if(0x00 != mmc_write(toc_buf, 8192, argv[1], toc_start_blk_offset)) {
        fprintf(stderr, "update hdr fail");
        if(toc_buf) {
            free(toc_buf);
            toc_buf = NULL;
        }
        return -1;
    }

    dump_package_head(hdr);

    ///////////////////////////////////////////////////////////////////////////
    printf("update dtb over.\n");

    if(toc_buf) {
        free(toc_buf);
        toc_buf = NULL;
    }

    return 0;
}
