
AC_DEFUN([AX_CHECK_PROGRAM_INVOCATION_NAME], [
  dnl Check if we have program_invocation_name available from our libc
  AC_MSG_CHECKING([for program_invocation_name])
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
      [extern const char *program_invocation_name;],
      [return (int)program_invocation_name;]
      )],
    [AC_MSG_RESULT([yes])
     AC_DEFINE_UNQUOTED([HAVE_PROGRAM_INVOCATION_NAME], [$libc_has_program_invocation_name],
       [Define to 1, if the libc has program_invocation_name])],
    [AC_MSG_RESULT([no])]
  )
])
