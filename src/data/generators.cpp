
#include "data/generators.h"

static void generators_populate_table (struct table_t*);
static void generators_populate_column (struct column_t*);

void
generators_populate (struct schema_t *schema)
{
  size_t i;
  for (i = 0; i < schema->tables.n; ++i)
    {
      generators_populate_table(schema->tables.tables[i]);
    }
}

void
generators_populate_table (struct table_t *table)
{
  size_t i;
  for (i = 0; i < table->columns.n; ++i)
    {
      generators_populate_column(table->columns.columns[i]);
    }

  // TODO: foreign keys (override pool definitions)
}

void
generators_populate_column (struct column_t *column)
{
  if (column->pool.type == VALUEPOOL_NONE)
    {
      
    }
}
