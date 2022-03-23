//////////////////////////////////////////////////////////////////////////
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////
//
// Public API for PryonLite Keyword Spotter
//
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

///
/// @brief Handle to a decoder instance
///
typedef void* PryonLiteDecoderHandle;

///
/// @brief VAD State
///
typedef enum PryonLiteVadState
{
    PRYON_LITE_VAD_INACTIVE = 0,
    PRYON_LITE_VAD_ACTIVE = 1
} PryonLiteVadState;

///
/// @brief VAD Event
///
typedef struct PryonLiteVadEvent
{
    PryonLiteVadState vadState;   ///< VAD state (inactive/active)

    void* reserved;               ///< reserved for future use

} PryonLiteVadEvent;

///
/// @brief Status / Error codes
///
typedef enum PryonLiteError
{
    PRYON_LITE_ERROR_OK = 0,            ///< Ok
    PRYON_LITE_ERROR,                   ///< General Error
    PRYON_LITE_ERROR_INVALID_CONFIG,    ///< Invalid Configuration
    PRYON_LITE_ERROR_NOT_INIT,          ///< Not Initialized
    PRYON_LITE_ERROR_ALREADY_INIT,      ///< Already Initialized
    PRYON_LITE_ERROR_INVALID_PARAM,     ///< Invalid Parameter
    PRYON_LITE_ERROR_INSUFFICIENT_MEM,  ///< Insufficient memory for given model
    PRYON_LITE_ERROR_INTERNAL,          ///< Internal error
    PRYON_LITE_ERROR_MODEL_INCOMPATIBLE,///< Model type or version is not compatible with this build of the engine
    PRYON_LITE_ERROR_MODEL_CORRUPT,     ///< Model malformed
    PRYON_LITE_ERROR_MODEL_ALIGNMENT    ///< Model alignment error
} PryonLiteError;


///
/// @brief Metadata blob accompanying keyword detection events
///
typedef struct PryonLiteMetadataBlob
{
    int blobSize; // in bytes
    const char *blob;
} PryonLiteMetadataBlob;

///
/// @brief Detection Result
///
typedef struct PryonLiteResult
{
    long long beginSampleIndex; ///< Identifies the sample index in the client's source of audio at which the
                                /// speech for the enumResult begins.

    long long endSampleIndex;   ///< Identifies the sample index in the client's source of audio at which the
                                /// the enumResult ends.  The number of samples in the audio for the
                                /// enumResult is given by endSampleIndex - beginSampleIndex.

    const char* keyword;        ///< The keyword that was detected

    PryonLiteMetadataBlob metadataBlob;  ///< Auxiliary information

    void* reserved;             ///< reserved for future use

} PryonLiteResult;

///
/// @brief Callback function for detections.
/// @param handle [in] Handle to decoder instance
/// @param result [in] Keyword spotter detection event
/// @note Elements within the result struct are valid only for the scope of the callback function.
///
typedef void (*PryonLiteResult_Callback)(PryonLiteDecoderHandle handle, const PryonLiteResult* result);

///
/// @brief Callback function for VAD Event.
///        This function is called whenever the VAD state changes
/// @param handle [in] Handle to decoder instance
/// @param event [in] Voice activity detector event
/// @note Data supplied by the callback is not guaranteed to be valid outside the scope of the callback.
///
typedef void (*PryonLiteVad_Callback)(PryonLiteDecoderHandle handle, const PryonLiteVadEvent* event);

///
/// @brief Configuration parameters to be passed in during initialization
///
typedef struct PryonLiteDecoderConfig
{
    PryonLiteResult_Callback resultCallback;    ///< callback function for handling detection notifications
    PryonLiteVad_Callback vadCallback;          ///< callback function for handling VAD state change notifications
                                                ///  optional, only when useVad is true.
    int detectThreshold;                        ///< integer from 1-1000. Default is 500.
                                                ///< 1 = lowest threshold, most detections.
                                                ///< 1000 = highest threshold, fewest detections.
    bool useVad;                                ///< Controls use of voice-activity-detector pre-stage
    bool lowLatency;                            ///< Only valid for type 'U' models. Results in ~200ms lower detection
                                                ///< latency, at the cost of less accurate ww end indexes reported in
                                                ///< the detection callback
    const void* model;                          ///< Wakeword model data (loaded from disk or statically compiled in)
                                                ///< *** Note this memory must persist while the library is in use ***
                                                ///< *** Note this model must be 4 byte aligned ***
    size_t sizeofModel;                         ///< The total size of model binary (in bytes).
    char *decoderMem;                           ///< Instance memory supplied by client for the decoder, as per model attributes
    size_t sizeofDecoderMem;                    ///< Size of memory supplied for the decoder instance (in bytes).
    struct PryonLiteDnnAccelConfig* dnnAccel;   ///< Pointer to configuration structure for external DNN acceleration
    void* reserved;                             ///< reserved, set to NULL
} PryonLiteDecoderConfig;


/// @brief Default Configuration to be used to initialize PryonLiteConfig struct
///
/// resultCallback = NULL
/// detectThreshold = 500
/// useVad = false
/// model = NULL
/// sizeofModel = 0
/// decoderMem = NULL
/// sizeofDecoderMem = 0
/// dnnAccel = NULL
/// reserved = NULL
///
extern const PryonLiteDecoderConfig PryonLiteDecoderConfig_Default;

///
/// @brief PryonLite Engine Attributes
///
typedef struct PryonLiteEngineAttributes
{
    const char* engineVersion;  ///< PryonLite engine version.
    void* reserved;             ///< reserved, set to NULL

} PryonLiteEngineAttributes;

///
/// @brief PryonLite Model Attributes
///
typedef struct PryonLiteModelAttributes
{
    const char* modelVersion;   ///< PryonLite model version.
    size_t requiredDecoderMem;  ///< Memory in bytes required by a decoder using this model
    void* reserved;             ///< reserved, set to NULL

} PryonLiteModelAttributes;


///
/// @brief Initialization Configuration
///
typedef struct PryonLiteSessionInfo
{
    int samplesPerFrame;   ///< input frame size required for calls to PryonLiteDecoder_PushAudioSamples()
    PryonLiteEngineAttributes engineAttributes; ///< Attributes pertaining to the engine, independent of loaded model.
    PryonLiteModelAttributes  modelAttributes;  ///< Attributes pertaining to the loaded model.
    void* reserved;            ///< reserved for future use
} PryonLiteSessionInfo;

///
/// @brief Fetches the Pryon Lite engine attributes
/// @param engineAttributes Target attributes need to be filled
/// @return PRYON_LITE_ERROR_OK if the attributes are fetched successfully.
///
PryonLiteError PryonLite_GetEngineAttributes(PryonLiteEngineAttributes* engineAttributes);

///
/// @brief Get model attributes
/// @param model [in] Wakeword model data (loaded from disk or statically compiled in)
/// @param sizeofModel [in] The total size of model binary (in bytes).
/// @param modelAttributes [out] Model attributes
/// @return PRYON_LITE_ERROR_OK for success, otherwise a non-zero error code
///
PryonLiteError PryonLite_GetModelAttributes(const void *model, size_t sizeofModel, PryonLiteModelAttributes* modelAttributes);

///
/// @brief Initializes the decoder
/// @param config [in]   Decoder configuration
/// @param result [out]  Address of a struct that is filled with session/engine/model information
/// @param pHandle [out] Pointer to decoder handle for use in subsequent references to decoder object.
/// @return PRYON_LITE_ERROR_OK if successful, otherwise a non-zero error code
///
PryonLiteError PryonLiteDecoder_Initialize(const PryonLiteDecoderConfig* config, PryonLiteSessionInfo* result, PryonLiteDecoderHandle *pHandle);

///
/// @brief Submits audio to be decoded.
///
/// @param handle [in] Handle to decoder instance
/// @param samples [in] Buffer of audio to be processed. Single-channel / 16-bit / 16khz / PCM
/// @param sampleCount [in]  Number of samples. This must be equal to the frame size returned in the
///                          "samplesPerFrame" member of the result structure in the call to
///                          PryonLiteDecoder_Initialize().
///
/// @return PRYON_LITE_ERROR_OK if successful, otherwise a non-zero error code
///
PryonLiteError PryonLiteDecoder_PushAudioSamples(PryonLiteDecoderHandle handle, const short* samples, int sampleCount);

///
/// @brief Destroys the decoder. All internal state and buffers are flushed and reset. Any pending
///        wakeword detection events internally queued will invoke the detection callback. After this call,
///        the decoder must be reinitialized before resuming processing.
///
/// @param pHandle [in/out] Handle to decoder instance, reset to NULL if reset is successful
///
/// @return PRYON_LITE_ERROR_OK if successful, otherwise a non-zero error code
///
PryonLiteError PryonLiteDecoder_Destroy(PryonLiteDecoderHandle *pHandle);

///
/// @brief Checks if the decoder has been initialized
///
/// @param handle [in] Handle to decoder instance
///
/// @return 1 if initialized, 0 if not initialized
///
int PryonLiteDecoder_IsDecoderInitialized(PryonLiteDecoderHandle handle);


///
/// @brief Sets the detection threshold parameter
///
/// @param handle [in] Handle to decoder instance
/// @param keyword [in] Keyword for which to set the detection threshold; ex. "ALEXA".  Pass a NULL pointer to set for all keywords.
/// @param detectThreshold [in] Integer in range [1, 1000] with 1 being most permissive, and 1000 being least permissive.
///
/// @return PRYON_LITE_ERROR_OK if successful, otherwise a non-zero error code
///
/// @note The keyword string must be in upper case.
///
PryonLiteError PryonLiteDecoder_SetDetectionThreshold(PryonLiteDecoderHandle handle, const char* keyword, int detectThreshold);

#ifdef __cplusplus
} // extern "C"
#endif

