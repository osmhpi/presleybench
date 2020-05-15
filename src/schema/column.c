
#include "column.h"

#include <stdlib.h>

void
column_init (struct column_t *column)
{
  column->name = NULL;
}

void
column_fini (struct column_t *column)
{
  free(column->name);
  column->name = NULL;
}
