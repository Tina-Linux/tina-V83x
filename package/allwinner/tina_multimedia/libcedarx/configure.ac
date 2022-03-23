#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
## 支持宏使用
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_SRCDIR([version.h])
AC_CONFIG_HEADERS([config.h])

## 不需要生成静态库
AC_DISABLE_STATIC
AM_SILENT_RULES([yes])
## support as
AM_PROG_AS

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_AR
## use libtool
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_CPP

AC_ARG_ENABLE(xplayerdemo,
		[AC_HELP_STRING([--enable-xplayerdemo],[build xplayerdemo])],
		[ case "${enableval}" in
		  yes) xplayerdemo=true ;;
		  no)  xplayerdemo=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-xplayerdemo) ;;
	esac],
	[xplayerdemo=false])
 AM_CONDITIONAL([XPLAYERDEMO_ENABLE],  [test x$xplayerdemo = xtrue])

AC_ARG_ENABLE(xplayerdemo-de2,
		[AC_HELP_STRING([--enable-xplayerdemo-de2],[DE 2.0 layer control for xplayerdemo])],
		[ case "${enableval}" in
		  yes) xplayerdemo_de2=true ;;
		  no)  xplayerdemo_de2=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-xplayerdemo-de2) ;;
	esac],
	[xplayerdemo_de2=false])
 AM_CONDITIONAL([XPLAYERDEMO_DE2_ENABLE],  [test x${xplayerdemo_de2} = xtrue])
echo "${xplayerdemo_de2} TJDEBUG1"

 AC_ARG_ENABLE(aac_parser,
		[AC_HELP_STRING([--enable-aac-parser],[enable aac parser])],
		[ case "${enableval}" in
		  yes) aac_parser=true ;;
		  no)  aac_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-aac-parser) ;;
	esac],
	[aac_parser=false])
 AM_CONDITIONAL([AAC_PARSER_ENABLE],  [test x$aac_parser = xtrue])

 AC_ARG_ENABLE(mp3_parser,
		[AC_HELP_STRING([--enable-mp3-parser],[enable mp3 parser])],
		[ case "${enableval}" in
		  yes) mp3_parser=true ;;
		  no)  mp3_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-mp3-parser) ;;
	esac],
	[mp3_parser=false])
 AM_CONDITIONAL([MP3_PARSER_ENABLE],  [test x$mp3_parser = xtrue])

 AC_ARG_ENABLE(wav_parser,
		[AC_HELP_STRING([--enable-wav-parser],[enable wav parser])],
		[ case "${enableval}" in
		  yes) wav_parser=true ;;
		  no)  wav_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-wav-parser) ;;
	esac],
	[wav_parser=false])
 AM_CONDITIONAL([WAV_PARSER_ENABLE],  [test x$wav_parser = xtrue])

 AC_ARG_ENABLE(id3v2_parser,
		[AC_HELP_STRING([--enable-id3v2-parser],[enable id3v2 parser])],
		[ case "${enableval}" in
		  yes) id3v2_parser=true ;;
		  no)  id3v2_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-id3v2-parser) ;;
	esac],
	[id3v2_parser=false])
 AM_CONDITIONAL([ID3V2_PARSER_ENABLE],  [test x$id3v2_parser = xtrue])

 AC_ARG_ENABLE(flac_parser,
		[AC_HELP_STRING([--enable-flac-parser],[enable flac parser])],
		[ case "${enableval}" in
		  yes) flac_parser=true ;;
		  no)  flac_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-flac-parser) ;;
	esac],
	[flac_parser=false])
 AM_CONDITIONAL([FLAC_PARSER_ENABLE],  [test x$flac_parser = xtrue])

 AC_ARG_ENABLE(ape_parser,
		[AC_HELP_STRING([--enable-ape-parser],[enable ape parser])],
		[ case "${enableval}" in
		  yes) ape_parser=true ;;
		  no)  ape_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-ape-parser) ;;
	esac],
	[ape_parser=false])
 AM_CONDITIONAL([APE_PARSER_ENABLE],  [test x$ape_parser = xtrue])

 AC_ARG_ENABLE(amr_parser,
		[AC_HELP_STRING([--enable-amr-parser],[enable amr parser])],
		[ case "${enableval}" in
		  yes) amr_parser=true ;;
		  no)  amr_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-amr-parser) ;;
	esac],
	[amr_parser=false])
 AM_CONDITIONAL([AMR_PARSER_ENABLE],  [test x$amr_parser = xtrue])

 AC_ARG_ENABLE(ogg_parser,
		[AC_HELP_STRING([--enable-ogg-parser],[enable ogg parser])],
		[ case "${enableval}" in
		  yes) ogg_parser=true ;;
		  no)  ogg_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-ogg-parser) ;;
	esac],
	[ogg_parser=false])
 AM_CONDITIONAL([OGG_PARSER_ENABLE],  [test x$ogg_parser = xtrue])

 AC_ARG_ENABLE(ts_parser,
		[AC_HELP_STRING([--enable-ts-parser],[enable ts parser])],
		[ case "${enableval}" in
		  yes) ts_parser=true ;;
		  no)  ts_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-ts-parser) ;;
	esac],
	[ts_parser=false])
 AM_CONDITIONAL([TS_PARSER_ENABLE],  [test x$ts_parser = xtrue])

 AC_ARG_ENABLE(mkv_parser,
		[AC_HELP_STRING([--enable-mkv-parser],[enable mkv parser])],
		[ case "${enableval}" in
		  yes) mkv_parser=true ;;
		  no)  mkv_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-mkv-parser) ;;
	esac],
	[mkv_parser=false])
 AM_CONDITIONAL([MKV_PARSER_ENABLE],  [test x$mkv_parser = xtrue])

 AC_ARG_ENABLE(mov_parser,
		[AC_HELP_STRING([--enable-mov-parser],[enable mov parser])],
		[ case "${enableval}" in
		  yes) mov_parser=true ;;
		  no)  mov_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-mov-parser) ;;
	esac],
	[mov_parser=false])
 AM_CONDITIONAL([MOV_PARSER_ENABLE],  [test x$mov_parser = xtrue])

 AC_ARG_ENABLE(avi_parser,
		[AC_HELP_STRING([--enable-avi-parser],[enable avi parser])],
		[ case "${enableval}" in
		  yes) avi_parser=true ;;
		  no)  avi_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-avi-parser) ;;
	esac],
	[avi_parser=false])
 AM_CONDITIONAL([AVI_PARSER_ENABLE],  [test x$avi_parser = xtrue])

 AC_ARG_ENABLE(flv_parser,
		[AC_HELP_STRING([--enable-flv-parser],[enable flv parser])],
		[ case "${enableval}" in
		  yes) flv_parser=true ;;
		  no)  flv_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-flv-parser) ;;
	esac],
	[flv_parser=false])
 AM_CONDITIONAL([FLV_PARSER_ENABLE],  [test x$flv_parser = xtrue])

 AC_ARG_ENABLE(asf_parser,
		[AC_HELP_STRING([--enable-asf-parser],[enable asf parser])],
		[ case "${enableval}" in
		  yes) asf_parser=true ;;
		  no)  asf_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-asf-parser) ;;
	esac],
	[asf_parser=false])
 AM_CONDITIONAL([ASF_PARSER_ENABLE],  [test x$asf_parser = xtrue])

 AC_ARG_ENABLE(hls_parser,
		[AC_HELP_STRING([--enable-hls-parser],[enable hls parser])],
		[ case "${enableval}" in
		  yes) hls_parser=true ;;
		  no)  hls_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-hls-parser) ;;
	esac],
	[hls_parser=false])
 AM_CONDITIONAL([HLS_PARSER_ENABLE],  [test x$hls_parser = xtrue])

 AC_ARG_ENABLE(mpg_parser,
		[AC_HELP_STRING([--enable-mpg-parser],[enable mpg parser])],
		[ case "${enableval}" in
		  yes) mpg_parser=true ;;
		  no)  mpg_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-mpg-parser) ;;
	esac],
	[mpg_parser=false])
 AM_CONDITIONAL([MPG_PARSER_ENABLE],  [test x$mpg_parser = xtrue])

 AC_ARG_ENABLE(pls_parser,
		[AC_HELP_STRING([--enable-pls-parser],[enable pls parser])],
		[ case "${enableval}" in
		  yes) pls_parser=true ;;
		  no)  pls_parser=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-pls-parser) ;;
	esac],
	[pls_parser=false])
 AM_CONDITIONAL([PLS_PARSER_ENABLE],  [test x$pls_parser = xtrue])

 AC_ARG_ENABLE(file_stream,
		[AC_HELP_STRING([--enable-file-stream],[enable file stream])],
		[ case "${enableval}" in
		  yes) file_stream=true ;;
		  no)  file_stream=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-file-stream) ;;
	esac],
	[file_stream=false])
 AM_CONDITIONAL([FILE_STREAM_ENABLE],  [test x$file_stream = xtrue])

 AC_ARG_ENABLE(http_stream,
		[AC_HELP_STRING([--enable-http-stream],[enable http stream])],
		[ case "${enableval}" in
		  yes) http_stream=true ;;
		  no)  http_stream=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-http-stream) ;;
	esac],
	[http_stream=false])
 AM_CONDITIONAL([HTTP_STREAM_ENABLE],  [test x$http_stream = xtrue])

  AC_ARG_ENABLE(https_stream,
		[AC_HELP_STRING([--enable-https-stream],[enable https stream])],
		[ case "${enableval}" in
		  yes) https_stream=true ;;
		  no)  https_stream=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-https-stream) ;;
	esac],
	[https_stream=false])
 AM_CONDITIONAL([HTTPS_STREAM_ENABLE],  [test x$https_stream = xtrue])

  AC_ARG_ENABLE(rtsp_stream,
		[AC_HELP_STRING([--enable-rtsp-stream],[enable rtsp stream])],
		[ case "${enableval}" in
		  yes) rtsp_stream=true ;;
		  no)  rtsp_stream=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-rtsp-stream) ;;
	esac],
	[rtsp_stream=false])
 AM_CONDITIONAL([RTSP_STREAM_ENABLE],  [test x$rtsp_stream = xtrue])

AC_ARG_ENABLE(aac_muxer,
		[AC_HELP_STRING([--enable-aac-muxer],[enable aac muxer])],
		[ case "${enableval}" in
		  yes) aac_muxer=true ;;
		  no)  aac_muxer=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-aac-muxer) ;;
	esac],
	[aac_muxer=false])
 AM_CONDITIONAL([AAC_MUXER_ENABLE],  [test x$aac_muxer = xtrue])

 AC_ARG_ENABLE(ts_muxer,
		[AC_HELP_STRING([--enable-ts-muxer],[enable ts muxer])],
		[ case "${enableval}" in
		  yes) ts_muxer=true ;;
		  no)  ts_muxer=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-ts-muxer) ;;
	esac],
	[ts_muxer=false])
 AM_CONDITIONAL([TS_MUXER_ENABLE],  [test x$ts_muxer = xtrue])

 AC_ARG_ENABLE(mp4_muxer,
		[AC_HELP_STRING([--enable-mp4-muxer],[enable mp4 muxer])],
		[ case "${enableval}" in
		  yes) mp4_muxer=true ;;
		  no)  mp4_muxer=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-mp4-muxer) ;;
	esac],
	[mp4_muxer=false])
 AM_CONDITIONAL([MP4_MUXER_ENABLE],  [test x$mp4_muxer = xtrue])

 AC_ARG_ENABLE(only_audio,
		[AC_HELP_STRING([--enable-only-audio],[ enable only audio])],
		[ case "${enableval}" in
		  yes) only_audio=true ;;
		  no)  only_audio=false ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-only-audio) ;;
	esac],
	[only_audio=false])
 AM_CONDITIONAL([TEST_ONLY_ENABLE_AUDIO],  [test x$only_audio = xtrue])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h locale.h malloc.h memory.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/socket.h sys/time.h sys/timeb.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_STRUCT_TIMEZONE
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MKTIME
AC_FUNC_STRTOD
AC_CHECK_FUNCS([bzero clock_gettime gethostbyname gettimeofday memchr memmove memset mkdir select socket strcasecmp strchr strdup strerror strncasecmp strrchr strspn strstr strtol strtoul strtoull])

AC_CONFIG_FILES([Makefile
		xmetadataretriever/Makefile
		awrecorder/Makefile
		xplayer/Makefile
		demo/Makefile
		demo/muxtest/Makefile
		demo/xmetademo/Makefile
		demo/jpegdemo/Makefile
		demo/recoderdemo/Makefile
		demo/parserdemo/Makefile
		demo/demoVdecoder/Makefile
		demo/demoVencoder/Makefile
		demo/xplayerdemo/Makefile
		demo/demoOmxVenc/Makefile
		demo/demoOmxVdec/Makefile
		libcore/Makefile
		libcore/playback/Makefile
		libcore/common/Makefile
		libcore/stream/Makefile
		libcore/stream/customer/Makefile
		libcore/stream/file/Makefile
		libcore/stream/http/Makefile
		libcore/stream/udp/Makefile
		libcore/stream/tcp/Makefile
		libcore/stream/ssl/Makefile
		libcore/stream/aes/Makefile
		libcore/stream/rtsp/Makefile
		libcore/stream/base/Makefile
		libcore/parser/Makefile
		libcore/parser/ts/Makefile
		libcore/parser/flv/Makefile
		libcore/parser/avi/Makefile
		libcore/parser/mov/Makefile
		libcore/parser/mkv/Makefile
		libcore/parser/asf/Makefile
		libcore/parser/remux/Makefile
		libcore/parser/hls/Makefile
		libcore/parser/ogg/Makefile
		libcore/parser/mpg/Makefile
		libcore/parser/ape/Makefile
		libcore/parser/flac/Makefile
		libcore/parser/amr/Makefile
		libcore/parser/mp3/Makefile
		libcore/parser/aac/Makefile
		libcore/parser/wav/Makefile
		libcore/parser/id3v2/Makefile
		libcore/parser/pls/Makefile
		libcore/parser/base/Makefile
		libcore/parser/AwRawStream/Makefile
		libcore/muxer/Makefile
		libcore/muxer/aac/Makefile
		libcore/muxer/mp4/Makefile
		libcore/muxer/ts/Makefile
		libcore/muxer/base/Makefile
		libcore/base/Makefile
		])
AC_OUTPUT

#plugin compile
#		libcore/libplugin/Makefile
#		libcore/libplugin/rxx/Makefile
#		libcore/libplugin/rtp/Makefile
