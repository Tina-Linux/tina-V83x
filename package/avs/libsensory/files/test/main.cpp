#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <iostream>

#include <snsr.h>

#define CHUNK_SAMPLES 160

SnsrSession m_session;

static std::string getSensoryDetails(SnsrSession session, SnsrRC result) {
    std::string message;
    // It is recommended by Sensory to prefer snsrErrorDetail() over snsrRCMessage() as it provides more details.
    if (session) {
        message = snsrErrorDetail(session);
    } else {
        message = snsrRCMessage(result);
    }
    if (message.empty()) {
        message = "Unrecognized error";
    }
    return message;
}

static SnsrRC keyWordDetectedCallback(SnsrSession s, const char* key, void* userData)
{
    static int count = 0;
    SnsrRC result;
    const char* keyword;
    double begin;
    double end;
    result = snsrGetDouble(s, SNSR_RES_BEGIN_SAMPLE, &begin);
    if (result != SNSR_RC_OK) {
        std::cout << "keyWordDetectedCallbackFailed getbegin " << getSensoryDetails(s, result) << std::endl;
        return result;
    }

    result = snsrGetDouble(s, SNSR_RES_END_SAMPLE, &end);
    if (result != SNSR_RC_OK) {
        std::cout << "keyWordDetectedCallbackFailed getend " << getSensoryDetails(s, result) << std::endl;
        return result;
    }

    result = snsrGetString(s, SNSR_RES_TEXT, &keyword);
    if (result != SNSR_RC_OK) {
        std::cout << "keyWordDetectedCallbackFailed keywordRetrievalFailure " << getSensoryDetails(s, result) << std::endl;
        return result;
    }

	std::cout << "keyWordDetected! " << count++ << ",begin: " << (uint64_t)begin << " end:" << (uint64_t)end  << " (samples)"<<std::endl;
    return SNSR_RC_OK;
}
int snsr_release()
{
    snsrRelease(m_session);
    return 0;
}
int snsr_init(const char *model, int point)
{

    // Allocate the Sensory library handle
    SnsrRC result = snsrNew(&m_session);
    if (result != SNSR_RC_OK) {
    std::cout << "initFailed: snsrNew " << getSensoryDetails(m_session, result) << std::endl;
    exit(-1);
    }

    // Get the expiration date of the library
    const char* info = nullptr;
    result = snsrGetString(m_session, SNSR_LICENSE_EXPIRES, &info);
    if (result == SNSR_RC_OK && info) {
        // Will print "License expires on <date>"
        std::cout << info << std::endl;
    } else {
        std::cout << "Sensory library license does not expire." << std::endl;
    }

    // Check if the expiration date is near, then we should display a warning
    result = snsrGetString(m_session, SNSR_LICENSE_WARNING, &info);
    if (result == SNSR_RC_OK && info) {
        // Will print "License will expire in <days-until-expiration> days."
        std::cout << info << std::endl;
    } else {
        std::cout << "Sensory library license does not expire for at least 60 more days." << std::endl;
    }

    result = snsrLoad(m_session, snsrStreamFromFileName(model, "r"));
    if (result != SNSR_RC_OK) {
        std::cout << "initFailed: snsrLoad " << getSensoryDetails(m_session, result) << std::endl;
        exit(-1);
    }

	if(point > 0){
		int target;
		result = snsrGetInt(m_session, SNSR_OPERATING_POINT, &target);
		std::cout << "Sensory model default operating point " << std::to_string(target) << std::endl;

		result = snsrSet(m_session, ("operating-point=" + std::to_string(point)).c_str());
		if (result != SNSR_RC_OK) {
			std::cout <<"error" << getSensoryDetails(m_session, result) << std::endl;
		}

		result = snsrGetInt(m_session, SNSR_OPERATING_POINT, &target);
		std::cout << "Sensory model new operating point " << std::to_string(target) << std::endl;
	}

	// Setting the callback handler
    result = snsrSetHandler(
            m_session,
            SNSR_RESULT_EVENT,
            snsrCallback(keyWordDetectedCallback, nullptr, nullptr));

    if (result != SNSR_RC_OK) {
	std::cout << "setUpRuntimeSettingsFailed: setKeywordDetectionHandlerFailure " << getSensoryDetails(m_session, result) << std::endl;
        exit(-1);
    }

    /*
     * Turns off automatic pipeline flushing that happens when the end of the input stream is reached. This is an
     * internal setting recommended by Sensory when audio is presented to Sensory in small chunks.
     */
    result = snsrSetInt(m_session, SNSR_AUTO_FLUSH, 0);
    if (result != SNSR_RC_OK) {
	std::cout << "setUpRuntimeSettingsFailed: disableAutoPipelineFlushingFailed " << getSensoryDetails(m_session, result) << std::endl;
        exit(-1);
    }
    return 0;
}

struct wav_header {
	uint32_t riff_id;			/*00H ~ 03H*/	//"RIFF"
	uint32_t riff_sz;			/*04H ~ 07H*/
	uint32_t riff_fmt;			/*08H ~ 0BH*/	//"WAVE"
	uint32_t fmt_id;			/*0CH ~ 0FH*/	//"fmt "
	uint32_t fmt_sz;			/*10H ~ 13H*/	//PCM 16
	uint16_t audio_format;		/*14H ~ 15H*/	//PCM 1
	uint16_t num_channels;		/*16H ~ 17H*/	//PCM 1
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
	uint32_t data_id;
	uint32_t data_sz;
};

int main(int argc, char *argv[])
{
    const char *wav = nullptr;
    const char *model = nullptr;
	int point = -1;
    if(argc < 3) exit(-1);

	wav = argv[1];
	model = argv[2];
	if(argc == 4) point = atoi(argv[3]);

    snsr_init(model, point);

    //File
    printf("open: %s\n", wav);

    FILE *fp = fopen(wav, "r");
    if(fp == NULL){
        printf("fopen error %s\n",strerror(errno));
        return -1;
    }
    char *data;
	char *final_data = data;
	int re_test = 1;
    while(re_test-- > 0) {
		struct wav_header header;
		int bytes = fread((void*)&header, 1, sizeof(struct wav_header), fp);
		if(bytes != sizeof(struct wav_header)) exit(-1);

		printf("num_channels = %d\n", header.num_channels);
		printf("bits_per_sample = %d\n", header.bits_per_sample);
		printf("sample_rate = %d\n", header.sample_rate);
		printf("total samples = %d\n", header.data_sz/(header.num_channels*header.bits_per_sample/8));

		if(header.num_channels > 1) {
			printf("target wav channel must be 1!\n");
			exit(-1);
		}

		if(header.sample_rate != 16000) {
			printf("target wav sample rate must be 16000!\n");
			exit(-1);
		}
		int sample_size = header.bits_per_sample/8;
		final_data = data = (char*)malloc(CHUNK_SAMPLES * sample_size);
		if(sample_size == 4){
			final_data = (char*)malloc(CHUNK_SAMPLES * 2);
		}
        //fseek(fp, 44, SEEK_SET); //skip wav header
        int should_break = 1;
		int32_t static_readbytes = 0;
        while(should_break) {
            bytes = fread(data, 1, CHUNK_SAMPLES*sample_size, fp);
            if(bytes < 0) {
                printf("fread error %s\n",strerror(errno));
                exit(-1);
            }
            if(bytes == 0){
                printf("Reach the end of the file? fread bytes %d\n",bytes);
                should_break = 0;
                continue;
            }

			//process print
			static_readbytes += bytes;
			float process = (float)static_readbytes*100/(float)header.data_sz;
			printf("\r[%0.2f%%] ", process);
			fflush(stdout);

			if(sample_size == 4) {
				uint16_t *target_samples = (uint16_t*)final_data;
				uint32_t *orgin_samples = (uint32_t*)data;
				for(int i = 0; i < bytes/sample_size; i++){
					target_samples[i] = orgin_samples[i] >> 16;
				}
				bytes = bytes/2;
			}

            snsrSetStream(
                    m_session,
                    SNSR_SOURCE_AUDIO_PCM,
                    snsrStreamFromMemory(final_data, bytes, SNSR_ST_MODE_READ));
            SnsrRC result = snsrRun(m_session);
            switch (result) {
                case SNSR_RC_STREAM_END:
                    // Reached end of buffer without any keyword detections
                    break;
                case SNSR_RC_OK:
                    std::cout << "SNSR_RC_OK" << std::endl;
                    break;
                default:
                    // A different return from the callback function that indicates some sort of error
                std::cout << "detect: unexpectedReturn " << getSensoryDetails(m_session, result) << std::endl;
                exit(-1);
            }
            snsrClearRC(m_session);
        }
    }

    fclose(fp);
    snsr_release();
    return 0;
}
