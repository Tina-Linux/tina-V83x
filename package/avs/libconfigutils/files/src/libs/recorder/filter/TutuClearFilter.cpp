#include "utils/JsonUtils.h"

#include "recorder/filter/TutuClearFilter.h"

#define GET_VAL_BY_KEY_CHECKED(json, key, value) \
do { \
    if(!JsonUtils::json_object_object_get_ex(json, key, &value)) { \
        printf("get %s failed\n", key); \
        return -1; }\
}while(0)

namespace AW {

void TutuClearProviderWrapper::setProvider(std::shared_ptr<ProviderInterface> provider)
{
    m_provider = provider;
}

std::shared_ptr<ProviderInterface> TutuClearProviderWrapper::getProvider()
{
    return m_provider;
}

void TutuClearProviderWrapper::setConvertor(std::shared_ptr<ConvertorInterface> convertor)
{
    m_convertor = convertor;
}

std::shared_ptr<ConvertorInterface> TutuClearProviderWrapper::getConvertor()
{
    return m_convertor;
}

int TutuClearProviderWrapper::setChannelMap(std::shared_ptr<ProviderChannelMap> channel_map)
{
    if(m_convertor == nullptr) return -1;
    if(channel_map == nullptr) return -1;
    if(channel_map->getMaptoChannel() == -1) return 0;

    return m_convertor->setChannelMap(channel_map->getMapChannel(), channel_map->getMaptoChannel());
}

void TutuClearProviderWrapper::setProvideType(ProvideType type)
{
    m_type = type;
}

int TutuClearProviderWrapper::fetch(int samples)
{
    if(m_provider == nullptr || m_convertor == nullptr) {
        printf("provider or convertor is nullptr\n");
        return -1;
    }

    return m_provider->fetch(m_convertor, samples);
}

TutuClearProviderWrapper::ProvideType TutuClearProviderWrapper::getProvideType()
{
    return m_type;
}

std::shared_ptr<TutuClearFilter> TutuClearFilter::create()
{
    return std::shared_ptr<TutuClearFilter>(new TutuClearFilter());
}

TutuClearFilter::~TutuClearFilter()
{

}

void TutuClearFilter::tutu_release()
{
    TUTUClear_Release(&pTUTUClearObject);
}

int TutuClearFilter::tutu_init()
{
    // read configuration
    tTUTUClearConfig.uw32Version = TUTUCLEAR_VERSION;
    printf("Parsing %s\n", m_prm_file);
    TUTUClear_ParsePRMFile_QACT(m_prm_file,
                                &tTUTUClearConfig,
                                &tTUTUClearParam);

    printf("uw16FrameSz = %d\n", (int)tTUTUClearConfig.uw16FrameSz);
    printf("uw16MaxNumOfMic = %d\n", (int)tTUTUClearConfig.uw16MaxNumOfMic);
    printf("uw16MaxTailLength = %d\n", (int)tTUTUClearConfig.uw16MaxTailLength);
    printf("uw16SamplingFreq = %d\n", (int)tTUTUClearConfig.uw16SamplingFreq);

    printf("w16MICSelection0 = 0x%04X\n", tTUTUClearParam.uw16Resv2);
    printf("w16MICSelection1 = 0x%04X\n", tTUTUClearParam.uw16Resv2);

    if (tTUTUClearConfig.uw16SamplingFreq*tTUTUClearConfig.uw16FrameSz > m_filter_block*1000)
    {
        printf("Illegal tTUTUClearConfig.\n");
        return -1;
    }

    // memeory allocation
    int iMemSzInByte = TUTUClear_QueryMemSz(&tTUTUClearConfig);
    printf("tutuClear DM usage = %d bytes\n", iMemSzInByte);
    pExternallyAllocatedMem = malloc(iMemSzInByte);

    // version check
    {
        W8 cMajor, cMinor, cRevision;
        UW32 uw32Config = TUTUClear_GetVerNum(&cMajor, &cMinor, &cRevision);
        fprintf(stderr, "Software voice processor compiled on: " __DATE__ " " __TIME__ "\n");
        fprintf(stderr, "TUTUCLEAR Ver. %d.%d.%d Inside (0x%08x).\n", cMajor, cMinor, cRevision, uw32Config);
        fprintf(stderr, "Copyright (C) 2017, Spectimbre Inc.\n");
    }

    // init
    printf("uw32OpMode = %08X\n", tTUTUClearParam.uw32OpMode);
    printf("uw32FuncMode = %08X\n", tTUTUClearParam.uw32FuncMode);
    printf("uw16NumOfMic = %d\n", tTUTUClearParam.uw16NumOfMic);
    printf("uw16ECTailLengthInMs = %d\n", tTUTUClearParam.uw16ECTailLengthInMs);
    if ((TUTUClear_Init(&tTUTUClearConfig,
                        pExternallyAllocatedMem,
                        &pTUTUClearObject)) != TUTU_OK)
    {
        printf("Fail to do TUTUClear_Init.\n");
        return -1;
    }
    else
        printf("TUTUClear_Init okay.\n");

    // set parameter
    if ((TUTUClear_SetParams(pTUTUClearObject, &tTUTUClearParam)) != TUTU_OK)
    {
        printf("Fail to do TUTUClear_SetParams.\n");
        return -1;
    }
    else
        printf("TUTUClear_SetParams okay.\n");
}

int TutuClearFilter::init(struct json_object *config)
{
    struct json_object *j_value;
    GET_VAL_BY_KEY_CHECKED(config, "prm-file", j_value);
    m_prm_file = JsonUtils::json_object_get_string(j_value);

    GET_VAL_BY_KEY_CHECKED(config, "filter-block", j_value);
    m_filter_block = JsonUtils::json_object_get_int(j_value);

    GET_VAL_BY_KEY_CHECKED(config, "input-sample-rate", j_value);
    m_input_sample_rate = JsonUtils::json_object_get_int(j_value);

    GET_VAL_BY_KEY_CHECKED(config, "input-channels", j_value);
    m_input_channels = JsonUtils::json_object_get_int(j_value);

    GET_VAL_BY_KEY_CHECKED(config, "input-sample-bits", j_value);
    m_input_sample_bits = JsonUtils::json_object_get_int(j_value);

    GET_VAL_BY_KEY_CHECKED(config, "output-sample-rate", j_value);
    m_output_sample_rate = JsonUtils::json_object_get_int(j_value);

    GET_VAL_BY_KEY_CHECKED(config, "output-channels", j_value);
    m_output_channels = JsonUtils::json_object_get_int(j_value);

    GET_VAL_BY_KEY_CHECKED(config, "output-sample-bits", j_value);
    m_output_sample_bits = JsonUtils::json_object_get_int(j_value);

    GET_VAL_BY_KEY_CHECKED(config, "signal-channel-0", j_value);
    m_signal_channel_0 = std::make_shared<ProviderChannelMap>(JsonUtils::json_object_get_string(j_value));

    GET_VAL_BY_KEY_CHECKED(config, "signal-channel-1", j_value);
    m_signal_channel_1 = std::make_shared<ProviderChannelMap>(JsonUtils::json_object_get_string(j_value));

    GET_VAL_BY_KEY_CHECKED(config, "signal-channel-2", j_value);
    m_signal_channel_2 = std::make_shared<ProviderChannelMap>(JsonUtils::json_object_get_string(j_value));

    //Signal Channels allways comes from same Provider
    if(strcmp(m_signal_channel_0->getProviderName(), m_signal_channel_1->getProviderName()) ||
       strcmp(m_signal_channel_1->getProviderName(), m_signal_channel_2->getProviderName()) ||
       strcmp(m_signal_channel_0->getProviderName(), m_signal_channel_2->getProviderName())) {
        printf("Signal Channels must comes from same Provider, now is not supported from different Provider\n");
    return -1;
    }
    m_signal_channel_0->setMaptoChannel(0);
    m_signal_channel_1->setMaptoChannel(1);
    m_signal_channel_2->setMaptoChannel(2);

    GET_VAL_BY_KEY_CHECKED(config, "reference-channel-0", j_value);
    m_reference_channel_0 = std::make_shared<ProviderChannelMap>(JsonUtils::json_object_get_string(j_value));

    GET_VAL_BY_KEY_CHECKED(config, "reference-channel-1", j_value);
    m_reference_channel_1 = std::make_shared<ProviderChannelMap>(JsonUtils::json_object_get_string(j_value));

    if(strcmp(m_reference_channel_0->getProviderName(), m_reference_channel_1->getProviderName()) &&
       strcmp(m_reference_channel_1->getProviderName(), "")) {
        printf("Reference Channels must comes from same Provider, now is not supported from different Provider\n");
        return -1;
    }

    m_signal_provider = std::make_shared<TutuClearProviderWrapper>();

    if(strcmp(m_signal_channel_0->getProviderName(), m_reference_channel_0->getProviderName()) == 0) {
        m_reference_channel_0->setMaptoChannel(3);
        m_reference_channel_1->setMaptoChannel(4);
    }else {
        m_reference_channel_0->setMaptoChannel(0);
        m_reference_channel_1->setMaptoChannel(1);

        m_reference_provider = std::make_shared<TutuClearProviderWrapper>();
    }

    if(strcmp(m_reference_channel_1->getProviderName(), "") == 0) {
        m_reference_channel_1->setMaptoChannel(-1);
    }

    printf("TutuClearFilter channel provider: %s, signal channel map:(%d->%d),(%d->%d),(%d->%d)\n",
                        m_signal_channel_0->getProviderName(),
                        m_signal_channel_0->getMapChannel(), m_signal_channel_0->getMaptoChannel(),
                        m_signal_channel_1->getMapChannel(), m_signal_channel_1->getMaptoChannel(),
                        m_signal_channel_2->getMapChannel(), m_signal_channel_2->getMaptoChannel());
    printf("TutuClearFilter channel provider: %s, reference channel map:(%d->%d),(%d->%d)\n",
                        m_reference_channel_0->getProviderName(),
                        m_reference_channel_0->getMapChannel(), m_reference_channel_0->getMaptoChannel(),
                        m_reference_channel_1->getMapChannel(), m_reference_channel_1->getMaptoChannel());

    m_tutuclear_outbuf = (char*)malloc(m_filter_block * m_output_channels *m_output_sample_bits/8);
    if(m_tutuclear_outbuf == nullptr) exit(-1);

    m_doa = DOAInfo::create();

    return tutu_init();

}

int TutuClearFilter::release()
{
    int ret = 0;
    tutu_release();
    if(m_signal_provider && m_signal_provider->getProvider()) {
        ret = m_signal_provider->getProvider()->release();
        if(ret > 0) return ret;
    }
    if(m_reference_provider && m_reference_provider->getProvider()) {
        ret = m_reference_provider->getProvider()->release();
        if(ret > 0) return ret;
    }

    return ret;
}

int TutuClearFilter::start()
{
    int ret = 0;
    if(m_signal_provider && m_signal_provider->getProvider()){
        ret = m_signal_provider->getProvider()->start();
        if(ret > 0) return ret;
    }
    if(m_reference_provider && m_reference_provider->getProvider()){
        ret = m_reference_provider->getProvider()->start();
        if(ret > 0) return ret;
    }

    return ret;
}

int TutuClearFilter::stop()
{
    int ret = 0;
    if(m_signal_provider && m_signal_provider->getProvider()){
        ret = m_signal_provider->getProvider()->stop();
        if(ret > 0) return ret;
    }
    if(m_reference_provider && m_reference_provider->getProvider()){
        ret = m_reference_provider->getProvider()->stop();
        if(ret > 0) return ret;
    }

    return ret;
}

int TutuClearFilter::fetch(std::shared_ptr<ConvertorInterface> convertor, int samples)
{
    if(samples != 0) {
        printf("TutuClearFilter only support fetch by block samples: %d\n", samples);
        return -1;
    }
    W32 *sig, *ref;
    int sample_fetch;
    int sample_factor;
    if(m_signal_provider != nullptr) {
        sample_factor = m_signal_provider->getProvider()->getSampleRate() / m_input_sample_rate;
        sample_fetch = m_signal_provider->fetch(sample_factor * m_filter_block);
        if(sample_fetch <= 0) return sample_fetch;

        m_signal_provider->getConvertor()->getChannelData(m_signal_channel_0->getMaptoChannel(), (char**)&sig);
    }
    //printf("%s %d\n", __func__,__LINE__);
    if(m_reference_provider != nullptr) {
        //printf("%s %d %d\n", __func__,__LINE__, m_reference_channel_0->getMaptoChannel());
        sample_factor = m_reference_provider->getProvider()->getSampleRate() / m_input_sample_rate;
        sample_fetch = m_reference_provider->fetch(sample_factor * m_filter_block);
        if(sample_fetch <= 0) return sample_fetch;

        m_reference_provider->getConvertor()->getChannelData(m_reference_channel_0->getMaptoChannel(), (char**)&ref);
    }else{
        //printf("%s %d %d\n", __func__,__LINE__, m_reference_channel_0->getMaptoChannel());
        m_signal_provider->getConvertor()->getChannelData(m_reference_channel_0->getMaptoChannel(), (char**)&ref);
    }
    //printf("%s %d\n", __func__,__LINE__);
    TUTUClear_OneFrame_32b(pTUTUClearObject,
                   ref, // aec reference signal
                   sig, // microphone signal
                   (W32*)m_tutuclear_outbuf, // processsing result
                   &tTUTUClearStat);
    //printf("%s %d\n", __func__,__LINE__);

    m_total_sample_count += m_filter_block;

    static int count = 0;
    if (tTUTUClearStat.w16VtrgrFlg != 0) {
        //printf("TutuClear buildin wakeup_word:\"alexa\" detected. count: %d\n", count++);
        //notifyKeyWordObservers("Alexa", m_total_sample_count - 0.7*m_output_sample_rate, m_total_sample_count);
    }
    notifyKeyWordObservers("Alexa", m_total_sample_count - 0.7*m_output_sample_rate, m_total_sample_count);

    double dbDOA = ((double)tTUTUClearStat.w16Resrv5*180.0) / (128.0*3.1415926);
    if (dbDOA < 0.0) dbDOA += 360.0;
    m_doa->set(dbDOA);

    return convertor->convert(m_tutuclear_outbuf, m_filter_block);
}

int TutuClearFilter::setProvider(std::shared_ptr<ProviderInterface> provider, std::shared_ptr<ConvertorInterface> convertor)
{
    if(m_signal_provider != nullptr && strcmp(m_signal_channel_0->getProviderName(), provider->getName()) == 0) {
        m_signal_provider->setProvider(provider);
        m_signal_provider->setConvertor(convertor);

        if(m_signal_provider->setChannelMap(m_signal_channel_0) < 0) return -1;
        if(m_signal_provider->setChannelMap(m_signal_channel_1) < 0) return -1;
        if(m_signal_provider->setChannelMap(m_signal_channel_2) < 0) return -1;

        if(m_reference_provider == nullptr) {
            if(m_signal_provider->setChannelMap(m_reference_channel_0) < 0) return -1;
            if(m_signal_provider->setChannelMap(m_reference_channel_1) < 0) return -1;
        }
    }else if(m_reference_provider != nullptr && strcmp(m_reference_channel_0->getProviderName(), provider->getName()) == 0) {
        m_reference_provider->setProvider(provider);
        m_reference_provider->setConvertor(convertor);

        if(m_reference_provider->setChannelMap(m_reference_channel_0) < 0) return -1;
        if(m_reference_provider->setChannelMap(m_reference_channel_1) < 0) return -1;
    }else {
        printf("setProvider: %s is not the target Provider, target: %s or %s\n",
                    provider->getName(),
                    m_signal_channel_0->getProviderName(),
                    m_reference_channel_0->getProviderName());

        return -1;
    }
    return 0;
}

TutuClearFilter::TutuClearFilter()
{

}

}
