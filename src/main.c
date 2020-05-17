
#include "main.h"

#include "schema/schema.h"

#include <errno.h>
#include <string.h>

extern const char *program_invocation_short_name;

static void
schema_dump (struct schema_t *schema)
{
  printf("schema {\n");

  size_t i;
  for (i = 0; i < schema->ntables; ++i)
    {
      struct table_t *table = schema->tables[i];
      printf("  table #%zu '%s' {\n", i, table->name);

      size_t j;
      for (j = 0; j < table->ncolumns; ++j)
        {
          struct column_t *column = table->columns[j];
          printf("    column #%zu '%s' {\n", j, column->name);

          printf("      type = ");
          switch (column->type.name)
            {
              case DATATYPE_INT:
                printf("int\n");
                break;
              case DATATYPE_SMALLINT:
                printf("smallint\n");
                break;
              case DATATYPE_NUMERIC:
                printf("numeric(%zu)\n", column->type.length);
                break;
              case DATATYPE_REAL:
                printf("real\n");
                break;
              case DATATYPE_FLOAT:
                printf("float\n");
                break;
              case DATATYPE_CHAR:
                printf("char(%zu)\n", column->type.length);
                break;
              case DATATYPE_DATETIME:
                printf("datetime\n");
                break;
              default:
                printf("<invalid>(%zu)\n", column->type.length);
                break;
            }

          printf("      primary key = %i\n", column->primary_key);

          printf("    }\n");
        }

      printf("  }\n");
    }

  printf("}\n");
}

int
main (int argc, char *argv[])
{
  // parse arguments
  struct arguments arguments = { NULL, 0 };
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  // parse schema
  struct schema_t schema;
  schema_init(&schema);

  int res = schema_parse_from_file(&schema, arguments.schemafile);
  if (res)
    {
      int errnum = errno;
      fprintf(stderr, "%s: %s: %s\n", program_invocation_short_name, arguments.schemafile, strerror(errnum));
      return res;
    }

  schema_dump(&schema);

  // cleanup
  schema_fini(&schema);

  return 0;
}
