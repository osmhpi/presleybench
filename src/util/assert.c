
#include "util/assert.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#if !HAVE_PROGRAM_INVOCATION_NAME
char *program_invocation_name = NULL;
#endif

int
assert_error_at_line (const char *filename, unsigned int lineno, const char* format, ...)
{
  int errnum = errno;

  int res = fprintf(stderr, "%s:%s:%u: error: ", program_invocation_name, filename, lineno);

  va_list args;
  va_start(args, format);
  res += vfprintf(stderr, format, args);
  va_end(args);

  if(errnum)
    res += fprintf(stderr, ": %s", strerror(errnum));

  res += fprintf(stderr, "\n");

  errno = errnum;

  return res;
}

int
assert_error (const char *format, ...)
{
  int errnum = errno;

  int res = fprintf(stderr, "%s: error: ", program_invocation_name);

  va_list args;
  va_start(args, format);
  res += vfprintf(stderr, format, args);
  va_end(args);

  if(errnum)
    res += fprintf(stderr, ": %s", strerror(errnum));

  res += fprintf(stderr, "\n");

  errno = errnum;

  return res;
}
