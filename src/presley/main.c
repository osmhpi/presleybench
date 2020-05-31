
#include "main.h"

#include "argparse.h"
#include "util/assert.h"
#include "schema/schema.h"
#include "data/generate.h"

#include <errno.h>
#include <string.h>

int
main (int argc, char *argv[])
{
  #if !HAVE_PROGRAM_INVOCATION_NAME
  program_invocation_name = argv[0];
  #endif

  // parse arguments
  int res;
  guard (0 == (res = argparse(argc, argv))) else
    {
      runtime_error("failed to comprehend command line arguments");
      return res;
    }

  // initialize schema structure
  struct schema_t schema;
  schema_init(&schema);

  // parse schema from file
  guard (0 == (res = schema_parse_from_file(&schema, arguments.schemafile))) else
    {
      runtime_error("%s: failed to parse schema", arguments.schemafile);
      return res;
    }

  // generate data
  guard (0 == (res = generate_schema_data(&schema))) else
    {
      runtime_error("%s: failed to generate data", arguments.schemafile);
      return res;
    }

  // cleanup
  schema_fini(&schema);

  return 0;
}
