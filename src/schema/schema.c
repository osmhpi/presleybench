
#include "schema/schema.h"

#include "schema/validate.h"

#include <stdlib.h>
#include <stdio.h>

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
  return list_add(&schema->_tables, table);
}

extern FILE *yyin;
const char *yyfilename;
extern void yylex_destroy(void);
extern int yyparse(struct schema_t*);

int
schema_parse_from_file (struct schema_t *schema, const char *filename)
{
  yyfilename = filename;

  yyin = fopen(filename, "r");
  if (!yyin)
    return 1;

  int res = yyparse(schema);

  fclose(yyin);
  yylex_destroy();

  if (res)
    return 1;

  // validate schema
  res = validate_schema(schema);
  if (res)
    return res;

  return 0;
}
