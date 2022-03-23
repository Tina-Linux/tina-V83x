#ifndef __CONFIG_UITLS_H__
#define __CONFIG_UITLS_H__
#include <json-c/json.h>
#include <string>
namespace Application{

class ConfigPropertyListener
{
public:
    virtual ~ConfigPropertyListener() = default;
    virtual void onConfigPropertyChanged(const char *porperty_key,
        const char *old_value, const char *new_value, const char *timespame) = 0;

};
class ConfigUtils
{
public:
    ConfigUtils() : configure(nullptr){};
    int init(const char *path);
    void release();

    const char *getPropertyValue(const char *ablum, const char *key);
    int setPropertyValue(const char *ablum, const char *key, const char *value);

    int setConfigPropertyListener(const char *ablum, const char *key, ConfigPropertyListener *l);
private:
    int save_configure();
    int reupdate_wifi_mac();
    int reupdate_device_id();
    int reupdate_bluetooth_mac();
private:
    struct json_object *configure;
    std::string configure_path;
};
}
#endif /*__CONFIG_UITLS_H__*/
