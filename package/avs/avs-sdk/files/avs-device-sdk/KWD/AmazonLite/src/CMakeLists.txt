add_definitions("-DACSDK_LOG_MODULE=pryonLiteKeywordDetector")

if(AMAZONLITE_KEY_WORD_DETECTOR_DYNAMIC_MODEL_LOADING)
	add_library(AMAZONLITE SHARED
		PryonLiteKeywordDetector.cpp)
else()
	add_library(AMAZONLITE SHARED
    	PryonLiteKeywordDetector.cpp "${AMAZONLITE_KEY_WORD_DETECTOR_MODEL_CPP_PATH}")
endif()

target_include_directories(AMAZONLITE PUBLIC
	"${AMAZONLITE_KEY_WORD_DETECTOR_INCLUDE_DIR}"
	"${KWD_SOURCE_DIR}/include" 
	"${AMAZONLITE_SOURCE_DIR}/include")

target_link_libraries(AMAZONLITE KWD AVSCommon "${AMAZONLITE_KEY_WORD_DETECTOR_LIB_PATH}")

# install target
asdk_install()