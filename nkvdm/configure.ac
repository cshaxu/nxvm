#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([nkvdm], [0.0], [cshaxu@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.
AC_CHECK_LIB([ncurses], [NCURSES_initscr], [have_ncurses=yes])
AC_CHECK_LIB([pthread], [PTHREAD_pthread_create], [have_pthread=yes])

if test "x${have_ncurses}" = xno; then
AC_MSG_ERROR([
------------------------------------------
Unable to find ncurses on this system.
------------------------------------------])
fi

if test "x${have_pthread}" = xno; then
AC_MSG_ERROR([
------------------------------------------
Unable to find pthread on this system.
------------------------------------------])
fi

# Checks for header files.
AC_CHECK_HEADERS([memory.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
