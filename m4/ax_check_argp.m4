
AC_DEFUN([AX_CHECK_ARGP], [
  dnl Check if we have argp available from our libc
  AC_MSG_CHECKING([for argp_parse in libc])
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
      [#include <argp.h>],
      [int argc=1; char *argv[]={"test"}; argp_parse(0,argc,argv,0,0,0); return 0;]
      )],
    [libc_has_argp="true"; AC_MSG_RESULT([yes])],
    [libc_has_argp="false"; AC_MSG_RESULT([no])]
  )
  dnl If our libc doesn't provide argp, then test for libargp
  if test "$libc_has_argp" = "false" ; then
    AC_CHECK_LIB([argp], [argp_parse], [have_argp="true"], [have_argp="false"])
    if test "$have_argp" = "false"; then
      AC_MSG_ERROR("no libargp found")
    else
      argp_LIBS="-largp"
    fi
  else
    argp_LIBS=""
  fi
  AC_SUBST([argp_LIBS])
])
