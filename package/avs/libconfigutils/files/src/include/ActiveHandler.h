#ifndef __ACTIVE_HANDLER_H__
#define __ACTIVE_HANDLER_H__

namespace AW {

class ActiveHandler
{
public:
    virtual int active() = 0;
    virtual int disactive() = 0;
};
}
#endif
