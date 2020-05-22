
#include "schema/table.h"

#include <stdlib.h>

void
table_init (struct table_t *table)
{
  table->name = NULL;
  table->rows = 0;

  list_init(&table->_columns);
  list_init(&table->_fks);
}

void
table_fini (struct table_t *table)
{
  free(table->name);
  table->name = NULL;

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
