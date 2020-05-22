
#include "main.h"

#include "argparse.h"
#include "schema/schema.h"
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
  argparse(argc, argv);

  // initialize schema structure
  struct schema_t schema;
  schema_init(&schema);

  // parse schema from file
  int res = schema_parse_from_file(&schema, arguments.schemafile);
  if (res)
    {
      fprintf(stderr, "%s: %s: %s\n", program_invocation_name, arguments.schemafile, strerror(errno));
      return res;
    }

  // generate data
  res = generate_schema_data(&schema);
  if (res)
    return res;

  // cleanup
  schema_fini(&schema);

  return 0;
}
