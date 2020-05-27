
AC_DEFUN([AX_ALLOW_DEBUG], [
  AC_ARG_ENABLE(debug,
    AS_HELP_STRING([--enable-debug], [Enable output of additional dignostic messages. Useful for debugging, and for submitting bug reports. Default: no]),
    [case "${enableval}" in
      yes) debug=1 ;;
      no)  debug=0 ;;
      *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
    esac],
    [debug=0])
  AC_DEFINE_UNQUOTED([DEBUG], [$debug], [increased debug output])
])
