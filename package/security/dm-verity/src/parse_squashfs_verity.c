#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "squashfs_verity.h"

static void sunxi_dump(char *buf, int ttl_len)
{
	int len;
	for (len = 0; len < ttl_len; len++) {
		printf("0x%02x ", ((char *)buf)[len]);
		if (len % 8 == 7) {
			printf("\n");
		}
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	struct verity_header verity_hdr;
	struct squashfs_super_block *squashfs_header = NULL;

	int fd_in = 0;
	int fd_sign = 0;
	int fd_root_hash = 0;
	int fd_hash_tree = 0;

	int read_len, write_len;
	int rootfs_size = 0;
	long long hash_tree_offset = 0;
	char hash_tree_offset_str[256] = {0};
	int ret = -1;

	if (argc != 5) {
		printf("Usage: extract_squashfs <squashfs_file> <sign> <root_hash> <hash_tree_offset>\n");
		goto out;
	}

	/* 1. open all input files*/
	fd_in = open(argv[1], O_RDONLY);
	if(fd_in == -1) {
		printf("Could not open squashfs_file: %s - %s\n", argv[1], strerror(errno));
		goto out;
	}

	fd_sign = open(argv[2], O_CREAT | O_RDWR | O_TRUNC);
	if (fd_sign == -1) {
		printf("Could not open sign file: %s - %s\n", argv[2], strerror(errno));
		goto out;
	}

	fd_root_hash = open(argv[3], O_CREAT | O_RDWR | O_TRUNC);
	if (fd_root_hash == -1) {
		printf("Could not open root_hash file: %s - %s\n", argv[3], strerror(errno));
		goto out;
	}

	fd_hash_tree = open(argv[4], O_CREAT | O_RDWR | O_TRUNC);
	if (fd_hash_tree == -1) {
		printf("Could not open hash_tree file: %s - %s\n", argv[4], strerror(errno));
		goto out;
	}

	/* 1. check the squashfs magic */
	if (lseek(fd_in, 0, SEEK_SET) == -1) {
		goto out;
	}

	squashfs_header = (struct squashfs_super_block *)malloc(sizeof(struct squashfs_super_block));
	if (squashfs_header == NULL) {
		printf("malloc squashfs_header faile\n");
		goto out;
	}

	read_len = read(fd_in, squashfs_header, sizeof(struct squashfs_super_block));
	if (read_len != sizeof(struct squashfs_super_block)) {
		goto out;
	}

	// sunxi_dump(squashfs_header, sizeof(struct squashfs_super_block));
	if (squashfs_header->s_magic != SQUASHFS_MAGIC) {
		printf("squashfs_file: %s magic error\n", argv[1]);
		goto out;
	}

	/* 2. get squashfs size, squashfs archives are often padded to 4KiB. */
	rootfs_size = (squashfs_header->bytes_used + ALIGNED_LEN - 1) & (~(0x1000 - 1));
#if 1
	/* For Tina, padding to 128K */
	rootfs_size = ((rootfs_size + (1 << 17) - 1) >> 17) << 17;
#endif

	/* 3. get verity_hdr */
	if (lseek(fd_in, rootfs_size, SEEK_SET) == -1) {
		goto out;
	}

	read_len = read(fd_in, &verity_hdr, sizeof(struct verity_header));
	if (read_len != sizeof(struct verity_header)) {
		goto out;
	}

	if ((verity_hdr.magic != VERITY_MAGIC_NUMBER) ||
		(verity_hdr.version != VERITY_VERSION))	{
		printf("error: verity header magic 0x%x should be 0x%x\n", verity_hdr.magic, VERITY_MAGIC_NUMBER);
		printf("error verity header version 0x%x should be 0x%x\n", verity_hdr.version, VERITY_VERSION);
		goto out;
	}

	/* 4. write sign and root_hash */
	write_len = write(fd_sign, verity_hdr.sign, VERITY_SIGN_LEN);
	if (write_len != VERITY_SIGN_LEN) {
		goto out;
	}

	write_len = write(fd_root_hash, verity_hdr.root_hash, verity_hdr.root_hash_size);
	if (write_len != verity_hdr.root_hash_size) {
		goto out;
	}

	/* 5. get the hash_tree_offset */
	hash_tree_offset = rootfs_size + sizeof(struct verity_header);
	if (hash_tree_offset % 512 != 0) {
		printf("Unsupported VERITY hash offset.\n");
		goto out;
	}

	read_len = sprintf(hash_tree_offset_str, "%lld", hash_tree_offset);

	write_len = write(fd_hash_tree, hash_tree_offset_str, read_len);
	if(write_len != read_len) {
		goto out;
	}

	ret = 0;

out:
	if (fd_in > 0)
		close(fd_in);
	if (fd_sign > 0)
		close(fd_sign);
	if (fd_root_hash > 0)
		close(fd_root_hash);
	if (fd_hash_tree > 0)
		close(fd_hash_tree);

	if (!squashfs_header)
		free(squashfs_header);

	return ret;
}
