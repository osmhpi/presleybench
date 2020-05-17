
#include "main.h"

#include "schema/schema.h"

#include <errno.h>
#include <string.h>

extern const char *program_invocation_short_name;

int
main (int argc, char *argv[])
{
  // parse arguments
  struct arguments arguments = { NULL, 0 };
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  // parse schema
  struct schema_t schema;
  int res = schema_parse_from_file(&schema, arguments.schemafile);
  if (res)
    {
      int errnum = errno;
      fprintf(stderr, "%s: %s: %s\n", program_invocation_short_name, arguments.schemafile, strerror(errnum));
      return res;
    }

  return 0;
}
