#ifndef STRINGLIST_H
#define STRINGLIST_H

#include <stddef.h>

struct stringlist_t
{
  size_t n;
  char **str;
};

void stringlist_init (struct stringlist_t*);

void stringlist_fini (struct stringlist_t*);

int stringlist_string_add (struct stringlist_t*, char*);

#endif
