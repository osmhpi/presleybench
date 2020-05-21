
#include "schema/stringlist.h"

#include <stdlib.h>

void
stringlist_init (struct stringlist_t *stringlist)
{
  stringlist->n = 0;
  stringlist->str = NULL;
}

void
stringlist_fini (struct stringlist_t *stringlist)
{
  free(stringlist->str);
  stringlist->str = NULL;
  stringlist->n = 0;
}

int
stringlist_string_add (struct stringlist_t *stringlist, char *str)
{
  char **strings = realloc(stringlist->str, sizeof(*strings) * (stringlist->n + 1));
  if (!strings)
    return 2;

  strings[stringlist->n] = str;

  stringlist->str = strings;
  stringlist->n++;

  return 0;
}
