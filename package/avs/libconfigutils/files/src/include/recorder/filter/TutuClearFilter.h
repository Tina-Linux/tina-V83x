#ifndef __TUTU_CLEAR_FILTER_H__
#define __TUTU_CLEAR_FILTER_H__

#include <list>

#include <tutu/tutuClear.h>
#include <tutu/tutu_tool.h>

#include "recorder/FilterInterface.h"

namespace AW {

class ProviderChannelMap
{
public:
    ProviderChannelMap(const char *config) {
        m_name = FilterInterface::GetProviderChannelbyString(config, m_map_channel);
        if(m_name == nullptr) {
            m_name = (char*)malloc(1);
            m_name[0] = 0;
        }
    };
    ~ProviderChannelMap() {
        if(m_name) free(m_name);
    };

    const char* getProviderName(){ return m_name; };
    int getMapChannel() { return m_map_channel; };
    int getMaptoChannel() { return m_map_to_channel; };

    int setMaptoChannel(int map_to_channel) { m_map_to_channel = map_to_channel; };

private:
    char *m_name{nullptr};
    int m_map_channel{-1};
    int m_map_to_channel{-1};
};
class TutuClearProviderWrapper
{
public:
    enum class ProvideType
    {
        SIGNAL,
        REFERENEC,
        BOTH,
        NONE
    };
    void setProvider(std::shared_ptr<ProviderInterface> provider);
    std::shared_ptr<ProviderInterface> getProvider();

    void setConvertor(std::shared_ptr<ConvertorInterface> convertor);
    std::shared_ptr<ConvertorInterface> getConvertor();

    int setChannelMap(std::shared_ptr<ProviderChannelMap> channel_map);

    void setProvideType(ProvideType type);
    ProvideType getProvideType();

    int fetch(int samples);
private:
    std::shared_ptr<ProviderInterface> m_provider{nullptr};
    std::shared_ptr<ConvertorInterface> m_convertor{nullptr};

    ProvideType m_type{ProvideType::NONE};
};

class TutuClearFilter : public FilterInterface
{
public:
    static std::shared_ptr<TutuClearFilter> create();
    ~TutuClearFilter();

    int init(struct json_object *config);
    int start();
    int stop();
    int release();
    int fetch(std::shared_ptr<ConvertorInterface> convertor, int samples);
    int setProvider(std::shared_ptr<ProviderInterface> provider, std::shared_ptr<ConvertorInterface> convertor);

private:
    TutuClearFilter();
    int tutu_init();
    void tutu_release();
    //tutu
    void                    *pTUTUClearObject = nullptr;
    void                    *pExternallyAllocatedMem = nullptr;
    TUTUClearConfig_t       tTUTUClearConfig;
    TUTUClearParam_t        tTUTUClearParam;
    TUTUClearStat_t         tTUTUClearStat;

    const char *m_prm_file{nullptr};

    std::shared_ptr<ProviderChannelMap> m_signal_channel_0{nullptr};
    std::shared_ptr<ProviderChannelMap> m_signal_channel_1{nullptr};
    std::shared_ptr<ProviderChannelMap> m_signal_channel_2{nullptr};

    std::shared_ptr<ProviderChannelMap> m_reference_channel_0{nullptr};
    std::shared_ptr<ProviderChannelMap> m_reference_channel_1{nullptr};

    std::shared_ptr<TutuClearProviderWrapper> m_signal_provider{nullptr};
    std::shared_ptr<TutuClearProviderWrapper> m_reference_provider{nullptr};

    char *m_tutuclear_outbuf{nullptr};

    uint64_t m_total_sample_count{0};
};
}
#endif
