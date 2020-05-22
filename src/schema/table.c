
#include "schema/table.h"

#include <stdlib.h>
#include <string.h>

void
table_init (struct table_t *table)
{
  table->name = NULL;
  table->parent = NULL;

  table->rows = 0;

  list_init(&table->_columns);
  list_init(&table->_fks);
}

void
table_fini (struct table_t *table)
{
  free(table->name);
  table->name = NULL;
  table->parent = NULL;

  table->rows = 0;

  list_fini(&table->_columns, (void(*)(void*))&column_fini);
  list_fini(&table->_fks, (void(*)(void*))&foreignkey_fini);
}

int
table_column_add (struct table_t *table, struct column_t *column)
{
  return list_add(&table->_columns, column);
}

int
table_foreignkey_add (struct table_t *table, struct foreignkey_t *fk)
{
  return list_add(&table->_fks, fk);
}

struct column_t*
table_get_column_by_name (struct table_t *table, const char *name)
{
  size_t i;
  for (i = 0; i < table->columns.n; ++i)
    if (!strcmp(table->columns.columns[i]->name, name))
      return table->columns.columns[i];

  return NULL;
}
