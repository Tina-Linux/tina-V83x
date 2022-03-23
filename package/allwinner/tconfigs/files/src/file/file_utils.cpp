#include "tconfigs/file/file_utils.h"

#include <sys/stat.h>
#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace file {

const int FileUtils::kErrorReturnValue = -1;
const int FileUtils::kEofReturnValue = -2;

FileUtils::~FileUtils(void)
{
    Close();
}

int FileUtils::OpenRead(const std::string& file)
{
    file_ = fopen(file.c_str(), "rb");
    if (!file_) {
        TCLOG_ERROR("Fail to open file \"%s\" to read", file.c_str());
        return kErrorReturnValue;
    }
    file_name_ = file;
    return 0;
}

int FileUtils::OpenWrite(const std::string& file)
{
    file_ = fopen(file.c_str(), "wb");
    if (!file_) {
        TCLOG_ERROR("Fail to open file \"%s\" to write", file.c_str());
        return kErrorReturnValue;
    }
    file_name_ = file;
    return 0;
}

void FileUtils::Close(void)
{
    if (file_) {
        fclose(file_);
        file_ = NULL;
    }
}

int FileUtils::Write(const char* buf, int bytes)
{
    int ret = 0;
    if (!file_) {
        return kErrorReturnValue;
    }
    ret = fwrite(buf, 1, bytes, file_);
    if (ret < bytes) {
        TCLOG_ERROR("Error writing file: \"%s\"", file_name_.c_str());
        ret = kErrorReturnValue;
    }
    return ret;
}

int FileUtils::Read(char* buf, int bytes)
{
    int ret = 0;
    if (!file_) {
        return kErrorReturnValue;
    }
    ret = fread(buf, 1, bytes, file_);
    if (ret < bytes) {
        if (ferror(file_)) {
            TCLOG_ERROR("Error reading file: \"%s\"", file_name_.c_str());
            clearerr(file_);
            return kErrorReturnValue;
        }
        if (feof(file_)) {
            TCLOG_DEBUG("End of file: \"%s\"", file_name_.c_str());
            clearerr(file_);
            return kEofReturnValue;
        }
    }
    return ret;
}

int FileUtils::Seek(SeekBasePosition base_pos, long offset)
{
    if (!file_) {
        return kErrorReturnValue;
    }
    int whence;
    switch (base_pos) {
    case kBegin:
        whence = SEEK_SET;
        break;
    case kCurrent:
        whence = SEEK_CUR;
        break;
    case kEnd:
        whence = SEEK_END;
        break;
    default:
        return kErrorReturnValue;
    }
    return fseek(file_, offset, whence);
}

unsigned long FileUtils::FileSizeInBytes(void)
{
    struct stat buf;
    stat(file_name_.c_str(), &buf);
    return buf.st_size;
}

} // namespace file
} // namespace tconfigs
