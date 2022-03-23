#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__

#include <stdio.h>
#include <string>

namespace AW {

class FileUtils
{
public:
    FileUtils() = default;
    virtual ~FileUtils() = default;

    virtual int create(const std::string &file, const std::string& mode);
    virtual void release();
    virtual int size();
    virtual long tell();
    virtual int write(const char *buf, int len);
    virtual int read(char *buf, int len);
    virtual int seek(long offset, int whence);
    virtual int gets(char *buf, int len);
    virtual std::string path();
private:
    FILE *fp;
    std::string file;
};

}
#endif
