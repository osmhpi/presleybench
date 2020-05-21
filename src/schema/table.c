
#include "schema/table.h"

#include <stdlib.h>

void
table_init (struct table_t *table)
{
  table->name = NULL;
  table->rows = 0;
  table->columns = NULL;
  table->ncolumns = 0;
}

void
table_fini (struct table_t *table)
{
  free(table->name);
  table->name = NULL;

  size_t i;
  for (i = 0; i < table->ncolumns; ++i)
    {
      column_fini(table->columns[i]);
      free(table->columns[i]);
      table->columns[i] = NULL;
    }

  free(table->columns);
  table->columns = NULL;
  table->ncolumns = 0;
}

int
table_column_add (struct table_t *table, struct column_t *column)
{
  struct column_t **columns = realloc(table->columns, sizeof(*table->columns) * (table->ncolumns + 1));

  if (!columns)
    return 1;

  table->columns = columns;
  table->columns[table->ncolumns] = column;
  table->ncolumns++;

  return 0;
}

