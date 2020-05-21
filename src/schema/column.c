
#include "schema/column.h"

#include <stdlib.h>

void
column_init (struct column_t *column)
{
  column->name = NULL;
  column->type.name = DATATYPE_NONE;
  column->type.length = 0;
  column->primary_key = 0;
  column->pool = 0;
  column->poolref.type = REFERENCE_NONE;
  column->poolref.ref = NULL;
  column->poolarr.n = 0;
  column->poolarr.str = NULL;
}

void
column_fini (struct column_t *column)
{
  free(column->name);
  column->name = NULL;
}
