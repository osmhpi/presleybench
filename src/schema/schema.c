
#include "schema/schema.h"

#include "schema/validate.h"
#include "schema/dump.h"

#include "util/assert.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void
schema_init (struct schema_t *schema)
{
  list_init(&schema->_tables);
}

void
schema_fini (struct schema_t *schema)
{
  list_fini(&schema->_tables, (void(*)(void*))&table_fini);
}

int
schema_table_add (struct schema_t *schema, struct table_t *table)
{
  int res;
  debug_guard (0 == (res = list_add(&schema->_tables, table)));
  return res;
}

struct table_t*
schema_get_table_by_name (struct schema_t *schema, const char *name)
{
  size_t i;
  for (i = 0; i < schema->tables.n; ++i)
    if (!strcmp(schema->tables.tables[i]->name, name))
      return schema->tables.tables[i];

  return NULL;
}

extern FILE *yyin;
const char *yyfilename;
extern void yylex_destroy(void);
extern int yyparse(struct schema_t*);

static int att_warn_unused_result
yyparse_clean (struct schema_t *schema)
{
  int res = yyparse(schema);
  fclose(yyin);
  yylex_destroy();
  return res;
}

static void
schema_dump (struct schema_t *schema, const char *filename)
{
  size_t len = strlen(filename);

  char *debug_filename;
  guard (NULL != (debug_filename = malloc(sizeof(*debug_filename) * (len + 7)))) else { return; }

  snprintf(debug_filename, len + 7, ".%s.dump", filename);

  FILE *debug;
  debug_guard (NULL != (debug = fopen(debug_filename, "w")));

  if (debug)
    {
      dump_schema(schema, debug);
      fclose(debug);
    }

  free(debug_filename);
}

int
schema_parse_from_file (struct schema_t *schema, const char *filename)
{
  yyfilename = filename;

  // parse schema
  guard (NULL != (yyin = fopen(filename, "r"))) else { return 1; }

  int res;
  guard (0 == (res = yyparse_clean(schema))) else { return res; }

  // validate schema
  guard (0 == (res = validate_schema(schema))) else { return res; }

  // dump schema to file for debugging
  if (DEBUG)
    schema_dump(schema, filename);

  return 0;
}
