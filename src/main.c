
#include "main.h"

#include "schema/schema.h"
#include "schema/validate.h"

#include "data/generate.h"

#include <errno.h>
#include <string.h>

#if !HAVE_PROGRAM_INVOCATION_NAME
const char *program_invocation_name = NULL;
#endif

int
main (int argc, char *argv[])
{
  #if !HAVE_PROGRAM_INVOCATION_NAME
  program_invocation_name = argv[0];
  #endif

  // parse arguments
  struct arguments arguments = { NULL, 0, 1 };
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  // initialize schema
  struct schema_t schema;
  schema_init(&schema);

  schema.scale = arguments.scale;

  // parse schema
  int res = schema_parse_from_file(&schema, arguments.schemafile);
  if (res)
    {
      fprintf(stderr, "%s: %s: %s\n", program_invocation_name, arguments.schemafile, strerror(errno));
      return res;
    }

  // validate schema
  res = validate_schema(&schema);
  if (res)
    return res;

  // generate data
  res = generate_schema_data(&schema);
  if (res)
    return res;

  // cleanup
  schema_fini(&schema);

  return 0;
}
