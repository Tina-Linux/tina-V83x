#ifndef __AMIXER_UTILS_H__
#define __AMIXER_UTILS_H__

#include <string>

namespace AW{

class AmixerUtils
{
public:
    static int cset(const char *card, const char *iface, const char *name, const char *value);
    static int cset(const char *card, const char *iface, const char *name, int value);
    static int cset(const char *card, const char *iface, const char *name, bool value);

    static int cget(const char *card, const char *iface, const char *name, std::string &value);
    static int cget(const char *card, const char *iface, const char *name, int &value);
    static int cget(const char *card, const char *iface, const char *name, bool &value);

private:
    static int amixer_opreation(const char *card,
                                const char *iface,
                                const char *name,
                                const char *value,
                                int roflag,
                                int *get);
};
}
#endif
