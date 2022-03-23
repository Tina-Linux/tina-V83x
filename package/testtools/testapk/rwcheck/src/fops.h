#ifndef __FOPS_H
#define __FOPS_H

#include <stdbool.h>
#include "rwcheck.h"
bool fops_is_existed(const char *path);
int fops_create_file(char *in_path, char *out_path,
        unsigned long long size, unsigned long long buf_size);
int fops_fill_crc(char *path);

#endif
