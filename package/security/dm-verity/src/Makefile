
all: generate_squashfs_verity parse_squashfs_verity

squashfs_verity:
	$(CC) $(CFLAGS) $(LDFALGS) -o generate_squashfs_verity generate_squashfs_verity.c

parse_squashfs_verity:
	$(CC) $(CFLAGS) $(LDFALGS) -o parse_squashfs_verity parse_squashfs_verity.c

clean:
	rm -rf  parse_squashfs_verity generate_squashfs_verity
