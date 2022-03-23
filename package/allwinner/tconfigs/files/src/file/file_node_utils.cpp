#include "tconfigs/file/file_node_utils.h"

#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace file {

std::shared_ptr<FileNodeUtils> FileNodeUtils::Create(
        const std::string& file_node)
{
    return std::shared_ptr<FileNodeUtils>(new FileNodeUtils(file_node));
}

std::shared_ptr<FileNodeUtils> FileNodeUtils::Create(
        const std::string& file_node, Mode mode)
{
    return std::shared_ptr<FileNodeUtils>(new FileNodeUtils(file_node, mode));
}

FileNodeUtils::FileNodeUtils(const std::string& file_node)
    : file_node_(file_node)
{
}

FileNodeUtils::FileNodeUtils(const std::string& file_node, Mode mode)
    : file_node_(file_node),
      mode_(mode)
{
}

int FileNodeUtils::Write(const char* buf, int bytes)
{
    if (mode_ != Mode::kWrite) {
        TCLOG_ERROR("FileNodeUtils: Not in mode \"Write\"");
        return -1;
    }
    FileUtils utils;
    if (0 != utils.OpenWrite(file_node_)) {
        return -1;
    }
    return utils.Write(buf, bytes);
}

int FileNodeUtils::Read(char* buf, int bytes)
{
    if (mode_ != Mode::kRead) {
        TCLOG_ERROR("FileNodeUtils: Not in mode \"Read\"");
        return -1;
    }
    FileUtils utils;
    if (0 != utils.OpenRead(file_node_)) {
        return -1;
    }
    return utils.Read(buf, bytes);
}

int FileNodeUtils::Read(void)
{
    char buf[1];
    return Read(buf, 1);
}

} // namespace file
} // namespace tconfigs
