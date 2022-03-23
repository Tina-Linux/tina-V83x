#ifndef __TCONFIGS_AUDIO_TUTUCLEAR_TUTUCLEAR_WRAPPER_H__
#define __TCONFIGS_AUDIO_TUTUCLEAR_TUTUCLEAR_WRAPPER_H__

#include <cstdint>
#include <tutu/tutuClear.h>

namespace tconfigs {
namespace audio {

class TutuclearWrapper {
public:
    TutuclearWrapper(void) = default;
    ~TutuclearWrapper(void);

    int Init(const char* prm_file, const char* keyword_file);
    void Release(void);

    int ProcessOneFrame(int16_t* common_signal_buf, int16_t* reference_signal_buf,
            int16_t* output_signal_buf);
    int ProcessOneFrame(int32_t* common_signal_buf, int32_t* reference_signal_buf,
            int32_t* output_signal_buf);

    bool KeywordIsDetected(void);

private:
    TUTUClearConfig_t tTUTUClearConfig;
    TUTUClearParam_t tTUTUClearParam;
    TUTUClearStat_t tTUTUClearStat;
    void *pExternallyAllocatedMem = nullptr;
    void *pTUTUClearObject = nullptr;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_TUTUCLEAR_TUTUCLEAR_WRAPPER_H__ */
