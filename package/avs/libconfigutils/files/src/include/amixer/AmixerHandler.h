#ifndef __AMIXER_HANDLER_H__
#define __AMIXER_HANDLER_H__

#include <memory>
#include <json-c/json.h>

#include "ActiveHandler.h"

namespace AW {

class AmixerHandler : public ActiveHandler
{
public:
    static std::shared_ptr<AmixerHandler> create(struct json_object *config);
    ~AmixerHandler();

    int active();
    int disactive();

private:
    AmixerHandler(){};
    int init(struct json_object *config);
    int action(const char* cmd);

    struct json_object *m_config;
};
}
#endif
