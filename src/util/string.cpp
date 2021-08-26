
#include "util/string.h"

#include "util/assert.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !HAVE_ASPRINTF
int
asprintf (char **ret, const char *format, ...)
{
  va_list ap;

  int len;
  va_start(ap, format);
  debug_guard (0 <= (len = vsnprintf(NULL, 0, format, ap)));
  va_end(ap);

  if (len < 0)
    return -1;

  guard (NULL != (*ret = malloc(sizeof(**ret) * (len + 1)))) else { return -1; }

  va_start(ap, format);
  debug_guard (len == vsnprintf(*ret, len + 1, format, ap));
  va_end(ap);

  return len;
}
#endif

int
astrcat (char **buf, size_t *len, const char *format, ...)
{
  va_list ap;
  size_t _len = (len ? *len : strlen(*buf));

  int addlen;
  va_start(ap, format);
  debug_guard (0 <= (addlen = vsnprintf(NULL, 0, format, ap)));
  va_end(ap);

  if (addlen < 0)
    return -1;

  char *newstr;
  guard (NULL != (newstr = realloc(*buf, sizeof(**buf) * (_len + addlen + 1)))) else { return 2; }

  va_start(ap, format);
  debug_guard(0 <= (addlen = vsnprintf(newstr + _len, addlen + 1, format, ap)));
  va_end(ap);

  if (addlen < 0)
    {
      free(newstr);
      return -1;
    }

  *buf = newstr;
  if (len) *len = _len;
  return _len;
}

