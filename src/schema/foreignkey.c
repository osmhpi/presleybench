
#include "schema/foreignkey.h"

#include <stdlib.h>

void
foreignkey_init (struct foreignkey_t *foreignkey)
{
  foreignkey->lhs_table = NULL;
  foreignkey->rhs_table = NULL;

  columnlist_init(&foreignkey->lhs);
  columnlist_init(&foreignkey->rhs);
}

void
foreignkey_fini (struct foreignkey_t *foreignkey)
{
  columnlist_fini(&foreignkey->lhs);
  columnlist_fini(&foreignkey->rhs);
}

void
columnlist_init (struct columnlist_t *columnlist)
{
  columnlist->n = 0;
  columnlist->names = NULL;
  columnlist->cols = NULL;
}

void
columnlist_fini (struct columnlist_t *columnlist)
{
  size_t i;
  for (i = 0; i < columnlist->n; ++i)
    {
      free(columnlist->names[i]);
      columnlist->names[i] = NULL;
    }
  free(columnlist->names);
  columnlist->names = NULL;
  free(columnlist->cols);
  columnlist->cols = NULL;
  columnlist->n = 0;
}

int
columnlist_column_add (struct columnlist_t *columnlist, char *name)
{
  // this only adds the name to the list, not resolving it.
  char **names = realloc(columnlist->names, sizeof(*names) * (columnlist->n + 1));
  if (!names)
    return 2;

  struct column_t **cols = realloc(columnlist->cols, sizeof(*cols) * (columnlist->n + 1));
  if (!cols)
    {
      free(names);
      return 2;
    }

  names[columnlist->n] = name;
  cols[columnlist->n] = NULL;

  columnlist->names = names;
  columnlist->cols = cols;
  columnlist->n++;

  return 0;
}
