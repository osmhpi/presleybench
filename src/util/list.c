
#include "util/list.h"

#include "util/assert.h"

#include <stdlib.h>

void
list_init (struct list_t *list)
{
  list->n = 0;
  list->e = NULL;
}

void
list_fini (struct list_t *list, void(*e_fini)(void*))
{
  size_t i;
  for (i = 0; i < list->n; ++i)
    {
      if (e_fini)
        (*e_fini)(list->e[i]);
      free(list->e[i]);
      list->e[i] = NULL;
    }

  list_fini_shallow(list);
}

void
list_fini_shallow (struct list_t *list)
{
  free(list->e);
  list->e = NULL;
  list->n = 0;
}

int
list_add (struct list_t *list, void *e)
{
  void **tmp;
  guard (NULL != (tmp = realloc(list->e, sizeof(*tmp) * (list->n + 1)))) else { return 2; }

  tmp[list->n] = e;
  list->e = tmp;
  list->n++;

  return 0;
}
