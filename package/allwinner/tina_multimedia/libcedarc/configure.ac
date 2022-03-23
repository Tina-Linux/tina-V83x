#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])

## 支持宏使用
AM_INIT_AUTOMAKE([-Wall foreign])

AC_CONFIG_SRCDIR([openmax/omxcore/include/aw_omx_component.h])
AC_CONFIG_HEADERS([config.h])

## 不需要生成静态库
AC_DISABLE_STATIC
AM_SILENT_RULES([yes])
## support as
AM_PROG_AS

# Checks for programs.
AM_PROG_AR
AC_PROG_CXX
AC_PROG_CC
## use libtool
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h malloc.h memory.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
#AC_FUNC_MALLOC
AC_FUNC_MMAP
#AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor gettimeofday memchr memmove memset munmap pow select sqrt strchr strdup strerror strncasecmp strrchr strstr strtol])

AC_CONFIG_FILES([Makefile
				base/Makefile
				memory/Makefile
				vdecoder/Makefile
				vencoder/base/Makefile
				vencoder/Makefile
				openmax/Makefile
				openmax/omxcore/Makefile
				openmax/vdec/Makefile
				openmax/venc/Makefile
				])

AC_OUTPUT
