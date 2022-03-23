#ifndef __TCONFIGS_COMMON_TIME_H__
#define __TCONFIGS_COMMON_TIME_H__

#include <sys/time.h>

namespace tconfigs {
namespace common {

bool TimeInterval(const timeval* start, const timeval* end, timeval* interval);

bool TimeIsGreaterThanOrEqualTo(const timeval* src, const timeval* dst);
bool TimeIsLessThanOrEqualTo(const timeval* src, const timeval* dst);

bool FloatingPointToTimeval(double src, timeval* dst);

} // namespace common
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_COMMON_TIME_H__ */
