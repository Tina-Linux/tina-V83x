#!/bin/sh

tt_base="/base/production/testtrecorder"
video_encoding_format=`mjson_fetch ${tt_base}/video_encoding_format`
audio_encoding_format=`mjson_fetch ${tt_base}/audio_encoding_format`
muxer_format=`mjson_fetch ${tt_base}/muxer_format`
recorder_max_timeS=`mjson_fetch ${tt_base}/recorder_max_timeS`
recorder_max_files=`mjson_fetch ${tt_base}/recorder_max_files`
recorder_file_path=`mjson_fetch ${tt_base}/recorder_file_path`

! [ -d "${recorder_file_path}" ] \
	&& echo "No Found recorder file path: ${recorder_file_path}, quit!!" \
	&& exit 1

[ -z "${video_encoding_format}"] \
	&& echo "tinatest.json video encoding format info is NULL" \
	&& exit 1

[ -z "${audio_encoding_format}"] \
	&& echo "tinatest.json audio encoding format info is NULL" \
	&& exit 1

[ -z "${muxer_format}"] \
	&& echo "tinatest.json muxer format info is NULL" \
	&& exit 1

[ -z "${recorder_max_timeS}"] \
	&& echo "tinatest.json recorder max time(S) info is NULL" \
	&& exit 1

[ -z "${recorder_max_files}"] \
	&& echo "tinatest.json recorder max files info is NULL" \
	&& exit 1

__trecordertest video_encoding_format ${video_encoding_format} \
	audio_encoding_format ${audio_encoding_format} \
	muxer_format ${muxer_format} \
	recorder_max_timeS ${recorder_max_timeS} \
	recorder_max_files ${recorder_max_files} \
	recorder_file_path ${recorder_file_path}

[ $? = 0 ] || exit 1
