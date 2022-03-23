#ifndef __TCONFIGS_FILE_FILE_NODE_UTILS_H__
#define __TCONFIGS_FILE_FILE_NODE_UTILS_H__

#include <memory>
#include <string>

#include "tconfigs/file/file_utils.h"

namespace tconfigs {
namespace file {

class FileNodeUtils {
public:
    enum class Mode {
        kUnknown,
        kWrite,
        kRead
    };

    static std::shared_ptr<FileNodeUtils> Create(const std::string& file_node);
    static std::shared_ptr<FileNodeUtils> Create(const std::string& file_node, Mode mode);
    ~FileNodeUtils(void) = default;

    int Write(const char* buf, int bytes);
    int Read(char* buf, int bytes);
    int Read(void);     // read only 1 byte

    void set_file_node(const std::string& file_node) { file_node_ = file_node; }
    void set_mode(Mode mode) { mode_ = mode; }

    const std::string& file_node(void) const { return file_node_; }
    Mode mode(void) const { return mode_; }

private:
    FileNodeUtils(void) = delete;
    FileNodeUtils(const std::string& file_node);
    FileNodeUtils(const std::string& file_node, Mode mode);

    std::string file_node_;
    Mode mode_ = Mode::kUnknown;
};

} // namespace file
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_FILE_FILE_NODE_UTILS_H__ */
