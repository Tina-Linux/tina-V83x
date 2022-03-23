#ifndef __RECORDER_INTERFACE_H__
#define __RECORDER_INTERFACE_H__

#include "recorder/ConvertorInterface.h"
#include "recorder/FilterInterface.h"

namespace AW {

class RecorderInterface
{
public:
    virtual ~RecorderInterface() = default;
    virtual int init() = 0;
    virtual int release() = 0;
    virtual int fetch(std::shared_ptr<ConvertorInterface> &convertor, int samples) = 0;
    virtual std::shared_ptr<ConvertorInterface> fetch(int samples) = 0;
    virtual int start() = 0;
    virtual int stop() = 0;

    virtual std::shared_ptr<FilterInterface> getFilter() = 0;
};

} // namespace AW

#endif /*__RECORDER_INTERFACE_H__*/
