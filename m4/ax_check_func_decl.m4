
AC_DEFUN([AX_CHECK_FUNC_DECL], [
  AS_VAR_PUSHDEF([ac_var],[ac_cv_func_decl_$1])
  AC_CHECK_DECL([$1],
    [AS_VAR_SET(ac_var, yes)],
    [AS_VAR_SET(ac_var, no)],
    [$2]
  )
  AS_IF([test AS_VAR_GET(ac_var) != "no"],
    [AC_DEFINE(AS_TR_CPP([HAVE_$1]), [1], [Define to 1 if you have the `$1' function.])],
    []
  )
  AS_VAR_POPDEF([ac_var])
])
