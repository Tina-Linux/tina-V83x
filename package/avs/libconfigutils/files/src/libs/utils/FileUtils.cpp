#include <string.h>
#include <errno.h>

#include <iostream>
#include "utils/FileUtils.h"

namespace AW {

int FileUtils::create(const std::string &file, const std::string& mode)
{
    fp = fopen(file.data(), mode.data());
    if(fp == NULL) {
        std::cout << file << " open file error " << strerror(errno) << std::endl;
        return -1;
    }
    this->file = file;
    return 0;
}

void FileUtils::release()
{
    if(fp != NULL) fclose(fp);
    fp = NULL;
}

int FileUtils::write(const char *buf, int len)
{
    if(fp == NULL) return -1;
    return fwrite(buf, 1, len, fp);;
}

int FileUtils::read(char *buf, int len)
{
    if(fp == NULL) return -1;
    int ret = fread(buf, len, 1, fp);
    if(ret <= 0) return ret;
    return len*ret;
}

long FileUtils::tell()
{
    if(fp == NULL) return -1;
    return ftell(fp);
}

int FileUtils::size()
{
    if(fp == NULL) return -1;
    int pos = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int flen = ftell(fp);
    fseek(fp, pos, SEEK_SET);
    return flen;
}

int FileUtils::seek(long offset, int whence)
{
    if(fp == NULL) return -1;
    return fseek(fp, offset, whence);
}

int FileUtils::gets(char *buf, int len)
{
    char *get_buf = fgets(buf, len - 1, fp);
    if(get_buf == NULL) return -1;
    return strlen(get_buf);
}

std::string FileUtils::path()
{
    return file;
}

}
