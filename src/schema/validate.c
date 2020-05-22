
#include "schema/validate.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

extern const char *program_invocation_name;

static int validate_table (struct table_t *table);
static int validate_column (struct column_t *column);

int
validate_schema (struct schema_t *schema)
{
  size_t i;
  for (i = 0; i < schema->tables.n; ++i)
    {
      struct table_t *table = schema->tables.tables[i];
      if (!table->name)
        {
          int len = snprintf(NULL, 0, "table#%zu", i);
          if (len < 0)
            {
              fprintf(stderr, "%s: schema:table#%zu: %s\n", program_invocation_name, i, strerror(errno));
              return 1;
            }
          table->name = malloc(sizeof(*table->name) * (len + 1));
          if (!table->name)
            {
              fprintf(stderr, "%s: schema:table#%zu: %s\n", program_invocation_name, i, strerror(errno));
              return 2;
            }
          snprintf(table->name, len + 1, "table#%zu", i);
        }

      table->parent = schema;

      int res = validate_table(table);
      if (res)
        return res;
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
      if (!column->name)
        {
          int len = snprintf(NULL, 0, "column#%zu", i);
          if (len < 0)
            {
              fprintf(stderr, "%s: schema:%s:column#%zu: %s\n", program_invocation_name, table->name, i, strerror(errno));
              return 1;
            }
          column->name = malloc(sizeof(*column->name) * (len + 1));
          if (!column->name)
            {
              fprintf(stderr, "%s: schema:%s:column#%zu: %s\n", program_invocation_name, table->name, i, strerror(errno));
              return 2;
            }
          snprintf(column->name, len + 1, "column#%zu", i);
        }

      column->parent = table;

      int res = validate_column(column);
      if (res)
        return res;
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
