
#include "main.h"

#include "schema/schema.h"

#include <errno.h>
#include <string.h>

extern const char *program_invocation_name;

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
  struct arguments arguments = { NULL, 0 };
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  // parse schema
  struct schema_t schema;
  schema_init(&schema);

  int res = schema_parse_from_file(&schema, arguments.schemafile);
  if (res)
    {
      int errnum = errno;
      fprintf(stderr, "%s: %s: %s\n", program_invocation_name, arguments.schemafile, strerror(errnum));
      return res;
    }

  // cleanup
  schema_fini(&schema);

  return 0;
}
