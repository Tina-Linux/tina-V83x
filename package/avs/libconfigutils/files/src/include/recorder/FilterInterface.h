#ifndef __FILTER_INTERFACE_H__
#define __FILTER_INTERFACE_H__

#include <string.h>
#include <memory>
#include <unordered_set>
#include <mutex>

#include <json-c/json.h>

#include "doa/DOAInfo.h"
#include "recorder/ProviderInterface.h"

namespace AW {

class FilterInterface
{
public:
    static char* GetProviderChannelbyString(const char *config, int &channel) {
        char *rel = strstr(config, "-");
        if(rel == nullptr) return nullptr;
        channel = atoi(rel + 1);
        int len = rel - config;
        char *name = (char*)malloc(len+1);
        strncpy(name, config, len);
        name[len] = '\0';
        return name;
    };
    virtual ~FilterInterface() = default;

    virtual int init(struct json_object *config) = 0;
    virtual int start() = 0;
    virtual int stop() = 0;
    virtual int release() = 0;
    virtual int fetch(std::shared_ptr<ConvertorInterface> converter,
                      int samples) = 0;
    virtual int setProvider(std::shared_ptr<ProviderInterface> provider,
                std::shared_ptr<ConvertorInterface> converter) = 0;

    int getInputSampleRate() { return m_input_sample_rate; };
    int getInputSampleBits() { return m_input_sample_bits; };
    int getInputChannels() { return m_input_channels; };

    int getOutputSampleRate() { return m_output_sample_rate; };
    int getOutputSampleBits() { return m_output_sample_bits; };
    int getOutputChannels() { return m_output_channels; };

    int getFilterBlock(){ return m_filter_block; };

    std::shared_ptr<DOAInfo> getDOAInfo() { return m_doa; };

    class KeyWordObserver
    {
    public:
        virtual void onFilterKeyWordDetected(std::string keyword, long beginIndex, long endIndex) = 0;
    };
    void addKeyWordObserver(std::shared_ptr<KeyWordObserver> keyWordObserver) {
        std::lock_guard<std::mutex> lock(m_keyWordObserversMutex);
        m_keyword_observer_set.insert(keyWordObserver);
    };

    void removeKeyWordObserver(std::shared_ptr<KeyWordObserver> keyWordObserver) {
        std::lock_guard<std::mutex> lock(m_keyWordObserversMutex);
        m_keyword_observer_set.erase(keyWordObserver);
    };

protected:
    void notifyKeyWordObservers(
        std::string keyword,
        long beginIndex,
        long endIndex) const {
        std::lock_guard<std::mutex> lock(m_keyWordObserversMutex);
        for (auto keyWordObserver : m_keyword_observer_set) {
            keyWordObserver->onFilterKeyWordDetected(keyword, beginIndex, endIndex);
        }
    };

protected:
    int m_filter_block;

    int m_input_sample_rate;
    int m_input_channels;
    int m_input_sample_bits;

    int m_output_sample_rate;
    int m_output_channels;
    int m_output_sample_bits;

    std::shared_ptr<DOAInfo> m_doa;

private:
    /// Lock to protect m_keyWordObservers when users wish to add or remove observers
    mutable std::mutex m_keyWordObserversMutex;
    std::unordered_set<std::shared_ptr<KeyWordObserver>> m_keyword_observer_set;


};
}

#endif
