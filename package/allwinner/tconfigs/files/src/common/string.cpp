#include "tconfigs/common/string.h"

#include <stdio.h>

namespace tconfigs {
namespace common {

int StringToInt(const std::string& value_str)
{
    int result;
    sscanf(value_str.c_str(), "%d", &result);
    return result;
}

std::string IntToString(int value)
{
    char result[12];
    sprintf(result, "%d", value);
    return result;
}

} // namespace common
} // namespace tconfigs
