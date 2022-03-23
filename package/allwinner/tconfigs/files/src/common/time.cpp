#include "tconfigs/common/time.h"

#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace common {

bool TimeInterval(const timeval* start, const timeval* end, timeval* interval)
{
    if (end->tv_sec < start->tv_sec) {
        TCLOG_ERROR("End time is before start time");
        return false;
    }

    if (end->tv_usec < start->tv_usec) {
        interval->tv_sec = end->tv_sec - start->tv_sec - 1;
        interval->tv_usec = 1e6 + end->tv_usec - start->tv_usec;
    } else {
        interval->tv_sec = end->tv_sec - start->tv_sec;
        interval->tv_usec = end->tv_usec - start->tv_usec;
    }
    return true;
}

bool TimeIsGreaterThanOrEqualTo(const timeval* src, const timeval* dst)
{
    if (src->tv_sec < dst->tv_sec) {
        return false;
    } else if (src->tv_sec > dst->tv_sec) {
        return true;
    } else {
        if (src->tv_usec >= dst->tv_usec) {
            return true;
        } else {
            return false;
        }
    }
}

bool TimeIsLessThanOrEqualTo(const timeval* src, const timeval* dst)
{
    if (src->tv_sec < dst->tv_sec) {
        return true;
    } else if (src->tv_sec > dst->tv_sec) {
        return false;
    } else {
        if (src->tv_usec <= dst->tv_usec) {
            return true;
        } else {
            return false;
        }
    }
}

bool FloatingPointToTimeval(double src, timeval* dst)
{
    if (src <= 0) {
        return false;
    }
    time_t sec = static_cast<time_t>(src);
    suseconds_t usec =
        static_cast<suseconds_t>((src - static_cast<double>(sec)) * 1e6);
    dst->tv_sec = sec;
    dst->tv_usec = usec;
    return true;
}

} // namespace common
} // namespace tconfigs
