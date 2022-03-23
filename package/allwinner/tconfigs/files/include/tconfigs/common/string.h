#ifndef __TCONFIGS_COMMON_STRING_H__
#define __TCONFIGS_COMMON_STRING_H__

#include <string>

namespace tconfigs {
namespace common {

int StringToInt(const std::string& value_str);

std::string IntToString(int value);

} // namespace common
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_COMMON_STRING_H__ */
