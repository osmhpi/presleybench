
#include "schema/validate.h"

#include "util/assert.h"
#include "util/string.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static int validate_table (struct table_t *table) att_warn_unused_result;
static int validate_column (struct column_t *column) att_warn_unused_result;

int
validate_schema (struct schema_t *schema)
{
  size_t i;
  for (i = 0; i < schema->tables.n; ++i)
    {
      struct table_t *table = schema->tables.tables[i];

      int res;
      if (!table->name)
        {
          guard (0 <= (res = asprintf(&table->name, "table#%zu", i))) else { return 2; }
        }

      table->parent = schema;

      guard (0 == (res = validate_table(table))) else { return res; }
    }

  return 0;
}

int
validate_table (struct table_t *table)
{
  size_t i;
  for (i = 0; i < table->columns.n; ++i)
    {
      struct column_t *column = table->columns.columns[i];

      int res;
      if (!column->name)
        {
          guard (0 <= (res = asprintf(&column->name, "column#%zu", i))) else { return res; }
        }

      column->parent = table;

      guard (0 == (res = validate_column(column))) else { return res; }
    }

  for (i = 0; i < table->fks.n; ++i)
    {
      struct foreignkey_t *fk = table->fks.fks[i];

      fk->lhs_table = table;
    }

  return 0;
}

int
validate_column (struct column_t *column)
{
  if (column->type.name == DATATYPE_NONE)
    column->type.name = DATATYPE_SMALLINT;

  column->pool.column = column;

  return 0;
}
