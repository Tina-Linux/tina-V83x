#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([libsunxivop], [0.0.1], [])

AM_INIT_AUTOMAKE([-Wall foreign])

AC_CONFIG_SRCDIR([src/sunxi_vop.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([memset strerror strtoul])


AC_CONFIG_FILES([Makefile
				 src/Makefile
				 src/edid/Makefile
				 example/Makefile
				 ])
AC_OUTPUT
