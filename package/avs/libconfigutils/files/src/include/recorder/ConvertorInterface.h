#ifndef __CONOVERTOR_INTERFACE_H__
#define __CONOVERTOR_INTERFACE_H__

namespace AW {

class ConvertorInterface
{
public:
    virtual int convert(char *data, int samples) = 0;
    virtual int setChannelMap(int ogrigin_channel, int map_channel) = 0;
    virtual int getChannelData(int channel, char **data) = 0;

};

}
#endif
