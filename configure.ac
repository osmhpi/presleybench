
dnl process this file with autoconf to produce the configure script

AC_PREREQ([2.68])
AC_INIT([presleybench], [0.1], [wimi-polze@hpi.de])
AC_CONFIG_SRCDIR([src/presley/main.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([scripts])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_PROG_CC
AM_PROG_LEX
AC_PROG_YACC

AC_CHECK_LIB([numa], [numa_available], [numa_LIBS=-lnuma; AC_SUBST(numa_LIBS)])

AX_PTHREAD([], [AC_MSG_ERROR([missing pthread])])

AX_CHECK_FUNC_DECL([asprintf], [stdio.h])

AX_CHECK_ARGP
AX_CHECK_PROGRAM_INVOCATION_NAME

AX_GCC_FUNC_ATTRIBUTE([warn_unused_result])
AX_GCC_FUNC_ATTRIBUTE([format])
AX_GCC_FUNC_ATTRIBUTE([unused])

AX_ALLOW_DEBUG

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
