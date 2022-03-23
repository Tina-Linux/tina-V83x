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
	long long rootfs_size = 0;
	long long rootfs_size_calc = 0;
	unsigned char *hash_tree_buffer = NULL;
	int ret = 0;

	if (argc != 5) {
		printf("Usage: squashfs_verity <squashfs_file> <sign> <root_hash> <hash_tree>\n");
		return -1;
	}

	/* 1. open all input files*/
	fd_in = open(argv[1], O_RDWR);
	if(fd_in == -1) {
		printf("Could not open squashfs_file: %s - %s\n", argv[1], strerror(errno));
		return -1;
	}

	fd_sign = open(argv[2], O_RDONLY);
	if (fd_sign == -1) {
		printf("Could not open sign file: %s - %s\n", argv[2], strerror(errno));
		ret = -1;
		goto out;
	}

	fd_root_hash = open(argv[3], O_RDONLY);
	if (fd_root_hash == -1) {
		printf("Could not open root_hash file: %s - %s\n", argv[3], strerror(errno));
		ret = -1;
		goto out;
	}

	fd_hash_tree = open(argv[4], O_RDONLY);
	if (fd_hash_tree == -1) {
		printf("Could not open hash_tree file: %s - %s\n", argv[4], strerror(errno));
		ret = -1;
		goto out;
	}

	/* 1. check the squashfs magic */
	if (lseek(fd_in, 0, SEEK_SET) == -1) {
		ret = -1;
		goto out;
	}

	squashfs_header = (struct squashfs_super_block *)malloc(sizeof(struct squashfs_super_block));
	if (squashfs_header == NULL) {
		printf("malloc squashfs_header faile\n");
		ret = -1;
		goto out;
	}

	read_len = read(fd_in, squashfs_header, sizeof(struct squashfs_super_block));
	if (read_len != sizeof(struct squashfs_super_block)) {
		printf("squashfs_file: %s read len %d error!\n", argv[1], read_len);
		ret = -1;
		goto out;
	}

	// sunxi_dump(squashfs_header, sizeof(struct squashfs_super_block));
	if (squashfs_header->s_magic != SQUASHFS_MAGIC) {
		printf("squashfs_file: %s magic error\n", argv[1]);
		ret = -1;
		goto out;
	}

	/* 2. get squashfs size, squashfs archives are often padded to 4KiB. */
	rootfs_size_calc = (squashfs_header->bytes_used + ALIGNED_LEN - 1) & (~(0x1000 - 1));

#if 1
	/* For Tina, padding to 128K */
	rootfs_size_calc = ((rootfs_size_calc + (1 << 17) - 1) >> 17) << 17;
#endif

	rootfs_size = lseek(fd_in, 0, SEEK_END);
	if (rootfs_size != rootfs_size_calc) {
		printf("rootfs_size: %lld, rootfs_size_calc: %lld\n", rootfs_size, rootfs_size_calc);
		ret = -1;
		goto out;
	}

	/* 3. get the sign_len and content. */
	read_len = lseek(fd_sign, 0, SEEK_END);
	if (read_len != VERITY_SIGN_LEN) {
		printf("sign: %s len should be %d\n", argv[2], VERITY_SIGN_LEN);
		ret = -1;
		goto out;
	}

	lseek(fd_sign, 0, SEEK_SET);
	read_len = read(fd_sign, verity_hdr.sign, VERITY_SIGN_LEN);
	if (read_len != VERITY_SIGN_LEN) {
		printf("sign: %s read len %d error!\n", argv[2], read_len);
		ret = -1;
		goto out;
	}

	/* 4. get the root_hash size and content. */
	verity_hdr.root_hash_size = lseek(fd_root_hash, 0, SEEK_END);
	if (verity_hdr.root_hash_size > VERITY_ROOT_HASH_MAXLEN) {
		printf("root_hash: %s size should <= %d\n", argv[3], VERITY_ROOT_HASH_MAXLEN);
		ret = -1;
		goto out;
	}

	lseek(fd_root_hash, 0, SEEK_SET);
	read_len = read(fd_root_hash, verity_hdr.root_hash, verity_hdr.root_hash_size);
	if (read_len != verity_hdr.root_hash_size) {
		printf("root_hash: %s read len %d error!\n", argv[3], read_len);
		ret = -1;
		goto out;
	}

	/* 5. get the hash_tree size and contend */
	verity_hdr.hash_tree_size = lseek(fd_hash_tree, 0, SEEK_END);
	hash_tree_buffer = (unsigned char *)malloc(verity_hdr.hash_tree_size);
	if (hash_tree_buffer == NULL) {
		printf("malloc hash_tree buffer: %d failed\n", verity_hdr.hash_tree_size);
		ret = -1;
		goto out;
	}

	lseek(fd_hash_tree, 0, SEEK_SET);
	read_len = read(fd_hash_tree, hash_tree_buffer, verity_hdr.hash_tree_size);
	if (read_len != verity_hdr.hash_tree_size) {
		printf("hash_tree: %s read len %d error!\n", argv[4], read_len);
		ret = -1;
		goto out;
	}

	/* 6. set verity_hdr magic and version */
	verity_hdr.magic = VERITY_MAGIC_NUMBER;
	verity_hdr.version = VERITY_VERSION;

	/* 7. write verity_hdr and hash_tree to the end of squahsfs */
	if (write(fd_in, &verity_hdr, sizeof(struct verity_header)) == -1) {
		ret = -1;
		goto out;
	}

	write_len = write(fd_in, hash_tree_buffer, verity_hdr.hash_tree_size);
	if (write_len == -1) {
		ret = -1;
	}

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
	if (!hash_tree_buffer)
		free(hash_tree_buffer);

	return ret;
}
