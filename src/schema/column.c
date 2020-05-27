
#include "schema/column.h"

#include "util/assert.h"

#include <stdlib.h>

int
column_init (struct column_t *column)
{
  column->name = NULL;
  column->parent = NULL;

  column->type.name = DATATYPE_NONE;
  column->type.length = 0;

  int res;
  guard (0 == (res = valuepool_init(&column->pool, VALUEPOOL_NONE))) else { return res; }

  column->primary_key = 0;

  return 0;
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
