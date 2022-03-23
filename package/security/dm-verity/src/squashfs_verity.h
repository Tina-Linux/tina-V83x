typedef long long		squashfs_inode;
struct squashfs_super_block {
	unsigned int		s_magic;
	unsigned int		inodes;
	int			mkfs_time /* time of filesystem creation */;
	unsigned int		block_size;
	unsigned int		fragments;
	unsigned short		compression;
	unsigned short		block_log;
	unsigned short		flags;
	unsigned short		no_ids;
	unsigned short		s_major;
	unsigned short		s_minor;
	squashfs_inode		root_inode;
	long long		bytes_used;
	long long		id_table_start;
	long long		xattr_id_table_start;
	long long		inode_table_start;
	long long		directory_table_start;
	long long		fragment_table_start;
	long long		lookup_table_start;
};

#define ALIGNED_LEN		4096
#define SQUASHFS_MAGIC		0x73717368

/*
  verity_header format: (total size 4096)

  0                4                8               12                16
  +----------------+----------------+----------------+----------------+ 0
  |      magic     |    version     | root_hase_size | hash_tree_size |
  +----------------+----------------+----------------+----------------+ 16
  |                                                                   |
  |                               sign                                |
  |                                                                   |
  +-------------------------------------------------------------------+ 16 + 256
  |                              reserved                             |
  +-------------------------------------------------------------------+ 1024
  |                                                                   |
  |                              root_hash                            |
  |                                                                   |
  +-------------------------------------------------------------------+
*/

#define VERITY_MAGIC_NUMBER (0xb001b001)
#define VERITY_VERSION (0)
#define VERITY_SIGN_LEN (256)
#define VERITY_RESERVED (1024 - VERITY_SIGN_LEN - 16)
#define VERITY_ROOT_HASH_MAXLEN (1024)
struct verity_header {
	unsigned int magic;
	unsigned int version;
	int root_hash_size;
	int hash_tree_size;
	unsigned char sign[VERITY_SIGN_LEN];
	unsigned char reseved[VERITY_RESERVED];
	unsigned char root_hash[VERITY_ROOT_HASH_MAXLEN];
	unsigned char reseved1[2048];
};
