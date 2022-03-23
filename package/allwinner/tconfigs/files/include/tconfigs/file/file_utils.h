#ifndef __TCONFIGS_FILE_FILE_UTILS_H__
#define __TCONFIGS_FILE_FILE_UTILS_H__

#include <string>

namespace tconfigs {
namespace file {

class FileUtils {
public:
    enum SeekBasePosition {
        kBegin,
        kCurrent,
        kEnd
    };

    FileUtils(void) = default;
    virtual ~FileUtils(void);

    virtual int OpenRead(const std::string& file);  // open the file to read
    virtual int OpenWrite(const std::string& file); // open the file to write (not append)
    virtual void Close(void);
    virtual int Write(const char* buf, int bytes);
    virtual int Read(char* buf, int bytes);
    virtual int Seek(SeekBasePosition base_pos, long offset);

    unsigned long FileSizeInBytes(void);

    const std::string& file_name(void) const { return file_name_; }

    static const int kErrorReturnValue; // a generic error return value
    static const int kEofReturnValue;   // a return value meaning end of file

private:
    FILE* file_ = NULL;
    std::string file_name_;
};

} // namespace file
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_FILE_FILE_UTILS_H__ */
