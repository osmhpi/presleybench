#ifndef STRING_H
#define STRING_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "util/assert.h"

#include <stdarg.h>
#include <stdio.h>

#if !HAVE_ASPRINTF
int asprintf(char**, const char*, ...) _warn_unused_result _format(printf, 2, 3);
#endif

int astrcat(char**, size_t*, const char*, ...) _warn_unused_result _format(printf, 3, 4);

#endif
