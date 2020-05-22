
#include "schema/column.h"

#include <stdlib.h>

int
column_init (struct column_t *column)
{
  column->name = NULL;
  column->parent = NULL;

  column->type.name = DATATYPE_NONE;
  column->type.length = 0;

  return valuepool_init(&column->pool, VALUEPOOL_NONE);

  column->primary_key = 0;
}

void
column_fini (struct column_t *column)
{
  free(column->name);
  column->name = NULL;
  column->parent = NULL;

  column->type.name = DATATYPE_NONE;
  column->type.length = 0;

  valuepool_fini(&column->pool);

  column->primary_key = 0;
}
