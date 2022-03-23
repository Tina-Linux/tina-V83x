#ifndef __DUILITE_H__
#define __DUILITE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NO_ANDROID
#undef __ANDROID__
#endif

#if (!(defined DUILITE_CALL) || !(defined DUILITE_IMPORT_OR_EXPORT))
	#if defined _WIN32
		#if defined _WIN64
			#define DUILITE_CALL __stdcall
		#else
			#define DUILITE_CALL
		#endif

		#ifdef DUILITE_IMPLEMENTION
			#define DUILITE_IMPORT_OR_EXPORT __declspec(dllexport)
		#else
			#define DUILITE_IMPORT_OR_EXPORT __declspec(dllimport)
		#endif
	#elif defined __ANDROID__
		#define DUILITE_CALL
		#define DUILITE_IMPORT_OR_EXPORT
		#undef  JNIEXPORT
		#define JNIEXPORT __attribute ((visibility("default")))
	#elif defined __APPLE__
		#define DUILITE_CALL
		#define DUILITE_IMPORT_OR_EXPORT
	#elif defined __unix__
		#define DUILITE_CALL
		#define DUILITE_IMPORT_OR_EXPORT __attribute ((visibility("default")))
	#else
		#define DUILITE_CALL
		#define DUILITE_IMPORT_OR_EXPORT
	#endif
#endif

/* version */
#define DUILITE_MAJOR  	  0
#define DUILITE_MINOR     11
#define DUILITE_REVISION  0

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define DUILITE_VERSION_STRING TOSTRING(DUILITE_MAJOR) "." TOSTRING(DUILITE_MINOR) "." TOSTRING(DUILITE_REVISION)
#define DUILITE_VERSION_TEXT "AISpeech DUILite SDK V" DUILITE_VERSION_STRING

#define DUILITE_VERSION_NUM   (((DUILITE_MAJOR)%100)*10000 + ((DUILITE_MINOR)%100)*100 + ((DUILITE_REVISION)%100))


#define DUILITE_MSG_TYPE_JSON     0
#define DUILITE_MSG_TYPE_BINARY   1


// log level
#define DUILITE_LOG_LEVEL_VERBOSE     1
#define DUILITE_LOG_LEVEL_DEBUG       2
#define DUILITE_LOG_LEVEL_INFO        3
#define DUILITE_LOG_LEVEL_WARNING     4
#define DUILITE_LOG_LEVEL_ERROR       5

enum duilite_callback_type {
	DUILITE_CALLBACK_FESPA_WAKEUP = 0,
	DUILITE_CALLBACK_FESPA_DOA,
	DUILITE_CALLBACK_FESPA_BEAMFORMING,
	DUILITE_CALLBACK_FESPL_WAKEUP,
	DUILITE_CALLBACK_FESPL_DOA,
	DUILITE_CALLBACK_FESPL_BEAMFORMING,
	DUILITE_CALLBACK_FEND_BEAMFORMING,
	DUILITE_CALLBACK_FEND_POST,
	DUILITE_CALLBACK_fespCar_WAKEUP,
	DUILITE_CALLBACK_fespCar_BEAMFORMING
};

typedef int (*duilite_callback)(void *userdata, int type, char *msg, int len);

DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_library_load(char *cfg);
DUILITE_IMPORT_OR_EXPORT void DUILITE_CALL duilite_library_release();
DUILITE_IMPORT_OR_EXPORT const char *duilite_get_version(void);

struct duilite_vad;
DUILITE_IMPORT_OR_EXPORT struct duilite_vad * DUILITE_CALL duilite_vad_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_start(struct duilite_vad *vad, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_feed(struct duilite_vad *vad, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_stop(struct duilite_vad *vad);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_cancel(struct duilite_vad *vad);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_delete(struct duilite_vad *vad);

struct duilite_speexenc;
DUILITE_IMPORT_OR_EXPORT struct duilite_speexenc * DUILITE_CALL duilite_speexenc_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_speexenc_start(struct duilite_speexenc *speexenc, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_speexenc_feed(struct duilite_speexenc *speexenc, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_speexenc_stop(struct duilite_speexenc *speexenc);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_speexenc_delete(struct duilite_speexenc *speexenc);

struct duilite_echo;
DUILITE_IMPORT_OR_EXPORT struct duilite_echo * DUILITE_CALL duilite_echo_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_start(struct duilite_echo *echo, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_feed(struct duilite_echo *echo, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_stop(struct duilite_echo *echo);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_cancel(struct duilite_echo *echo);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_delete(struct duilite_echo *echo);

struct duilite_wakeup;
DUILITE_IMPORT_OR_EXPORT struct duilite_wakeup * DUILITE_CALL duilite_wakeup_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_start(struct duilite_wakeup *wakeup, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_feed(struct duilite_wakeup *wakeup, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_stop(struct duilite_wakeup *wakeup);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_cancel(struct duilite_wakeup *wakeup);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_set(struct duilite_wakeup *wakeup, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_delete(struct duilite_wakeup *wakeup);

struct duilite_cntts;
DUILITE_IMPORT_OR_EXPORT struct duilite_cntts * DUILITE_CALL duilite_cntts_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_cntts_start(struct duilite_cntts *cntts, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_cntts_feed(struct duilite_cntts *cntts, char *data);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_cntts_set(struct duilite_cntts *cntts, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_cntts_delete(struct duilite_cntts *cntts);

struct duilite_gram;
DUILITE_IMPORT_OR_EXPORT struct duilite_gram * DUILITE_CALL duilite_gram_new(char *cfg);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_gram_start(struct duilite_gram *gram, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_gram_delete(struct duilite_gram *gram);

struct duilite_asr;
DUILITE_IMPORT_OR_EXPORT struct duilite_asr * DUILITE_CALL duilite_asr_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_start(struct duilite_asr *asr, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_feed(struct duilite_asr *asr, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_stop(struct duilite_asr *asr);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_cancel(struct duilite_asr *asr);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_delete(struct duilite_asr *asr);

struct duilite_fespa;
DUILITE_IMPORT_OR_EXPORT struct duilite_fespa * DUILITE_CALL duilite_fespa_new(char *cfg);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_register(struct duilite_fespa *fespa, int callback_type, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_start(struct duilite_fespa *fespa, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_feed(struct duilite_fespa *fespa, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_stop(struct duilite_fespa *fespa);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_set(struct duilite_fespa *fespa, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_delete(struct duilite_fespa *fespa);

struct duilite_fespl;
DUILITE_IMPORT_OR_EXPORT struct duilite_fespl * DUILITE_CALL duilite_fespl_new(char *cfg);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_register(struct duilite_fespl *fespl, int callback_type, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_start(struct duilite_fespl *fespl, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_feed(struct duilite_fespl *fespl, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_stop(struct duilite_fespl *fespl);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_set(struct duilite_fespl *fespl, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_delete(struct duilite_fespl *fespl);

struct duilite_fdm;
DUILITE_IMPORT_OR_EXPORT struct duilite_fdm * DUILITE_CALL duilite_fdm_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_start(struct duilite_fdm *fdm, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_set(struct duilite_fdm *fdm, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_get(struct duilite_fdm *fdm, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_feed(struct duilite_fdm *fdm, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_stop(struct duilite_fdm *fdm);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_delete(struct duilite_fdm *fdm);

struct duilite_nr;
DUILITE_IMPORT_OR_EXPORT struct duilite_nr * DUILITE_CALL duilite_nr_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nr_start(struct duilite_nr *nr, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nr_feed(struct duilite_nr *nr, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nr_stop(struct duilite_nr *nr);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nr_delete(struct duilite_nr *nr);

struct duilite_vprint;
DUILITE_IMPORT_OR_EXPORT struct duilite_vprint * DUILITE_CALL duilite_vprint_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vprint_start(struct duilite_vprint *vprint, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vprint_feed(struct duilite_vprint *vprint, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vprint_stop(struct duilite_vprint *vprint);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vprint_delete(struct duilite_vprint *vprint);

struct duilite_qbye;
DUILITE_IMPORT_OR_EXPORT struct duilite_qbye * DUILITE_CALL duilite_qbye_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_qbye_start(struct duilite_qbye *qbye, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_qbye_feed(struct duilite_qbye *qbye, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_qbye_stop(struct duilite_qbye *qbye);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_qbye_delete(struct duilite_qbye *qbye);

struct duilite_gender;
DUILITE_IMPORT_OR_EXPORT struct duilite_gender * DUILITE_CALL duilite_gender_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_gender_start(struct duilite_gender *gender, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_gender_feed(struct duilite_gender *gender, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_gender_stop(struct duilite_gender *gender);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_gender_delete(struct duilite_gender *gender);

struct duilite_semantic;
DUILITE_IMPORT_OR_EXPORT struct duilite_semantic * DUILITE_CALL duilite_semantic_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_semantic_start(struct duilite_semantic *semantic, char *param);
//DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_semantic_feed(struct duilite_semantic *semantic, char *data, int len);
//DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_semantic_stop(struct duilite_semantic *semantic);
//DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_semantic_cancel(struct duilite_semantic *semantic);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_semantic_delete(struct duilite_semantic *semantic);

struct duilite_nwakeup;
DUILITE_IMPORT_OR_EXPORT struct duilite_nwakeup * DUILITE_CALL duilite_nwakeup_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nwakeup_start(struct duilite_nwakeup *nwakeup, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nwakeup_feed(struct duilite_nwakeup *nwakeup, int index, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nwakeup_stop(struct duilite_nwakeup *nwakeup);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nwakeup_cancel(struct duilite_nwakeup *nwakeup);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nwakeup_set(struct duilite_nwakeup *nwakeup, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nwakeup_delete(struct duilite_nwakeup *nwakeup);

struct duilite_dmasp;
DUILITE_IMPORT_OR_EXPORT struct duilite_dmasp * DUILITE_CALL duilite_dmasp_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_dmasp_start(struct duilite_dmasp *dmasp, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_dmasp_feed(struct duilite_dmasp *dmasp, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_dmasp_stop(struct duilite_dmasp *dmasp);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_dmasp_cancel(struct duilite_dmasp *dmasp);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_dmasp_delete(struct duilite_dmasp *dmasp);

struct duilite_fend;
DUILITE_IMPORT_OR_EXPORT struct duilite_fend * DUILITE_CALL duilite_fend_new(char *cfg);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fend_register(struct duilite_fend *fend, int callback_type, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fend_start(struct duilite_fend *fend, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fend_feed(struct duilite_fend *fend, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fend_stop(struct duilite_fend *fend);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fend_cancel(struct duilite_fend *fend);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fend_delete(struct duilite_fend *fend);

struct duilite_asrpp;
DUILITE_IMPORT_OR_EXPORT struct duilite_asrpp * DUILITE_CALL duilite_asrpp_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asrpp_start(struct duilite_asrpp *asrpp, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asrpp_feed(struct duilite_asrpp *asrpp, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asrpp_stop(struct duilite_asrpp *asrpp);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asrpp_cancel(struct duilite_asrpp *asrpp);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asrpp_delete(struct duilite_asrpp *asrpp);

struct duilite_agc;
DUILITE_IMPORT_OR_EXPORT struct duilite_agc * DUILITE_CALL duilite_agc_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_agc_start(struct duilite_agc *agc, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_agc_feed(struct duilite_agc *agc, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_agc_stop(struct duilite_agc *agc);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_agc_cancel(struct duilite_agc *agc);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_agc_delete(struct duilite_agc *agc);

struct duilite_audioCut;
DUILITE_IMPORT_OR_EXPORT struct duilite_audioCut * DUILITE_CALL duilite_audioCut_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_audioCut_start(struct duilite_audioCut *audioCut, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_audioCut_feed(struct duilite_audioCut *audioCut, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_audioCut_stop(struct duilite_audioCut *audioCut);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_audioCut_cancel(struct duilite_audioCut *audioCut);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_audioCut_delete(struct duilite_audioCut *audioCut);

struct duilite_fespCar;
DUILITE_IMPORT_OR_EXPORT struct duilite_fespCar * DUILITE_CALL duilite_fespCar_new(char *cfg);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespCar_register(struct duilite_fespCar *fespCar, int callback_type, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespCar_start(struct duilite_fespCar *fespCar, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespCar_feed(struct duilite_fespCar *fespCar, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespCar_stop(struct duilite_fespCar *fespCar);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespCar_set(struct duilite_fespCar *fespCar, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespCar_getDriveMode(struct duilite_fespCar *fespCar);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespCar_delete(struct duilite_fespCar *fespCar);

#ifdef __cplusplus
}
#endif

#endif
