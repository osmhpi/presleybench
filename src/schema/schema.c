
#include "schema.h"

#include <stdlib.h>

void
schema_init (struct schema_t *schema)
{
  schema->tables = NULL;
  schema->ntables = 0;
}

void
schema_fini (struct schema_t *schema)
{
  size_t i;
  for (i = 0; i < schema->ntables; ++i)
    {
      table_fini(schema->tables[i]);
      free(schema->tables[i]);
      schema->tables[i] = NULL;
    }

  free(schema->tables);
  schema->tables = NULL;
  schema->ntables = 0;
}

int
schema_table_add (struct schema_t *schema, struct table_t *table)
{
  struct table_t **tables = realloc(schema->tables, sizeof(*schema->tables) * (schema->ntables + 1));

  if (!tables)
    return 1;

  schema->tables = tables;
  schema->tables[schema->ntables] = table;
  schema->ntables++;

  return 0;
}
