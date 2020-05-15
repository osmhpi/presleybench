
#include "main.h"

#include "schema/schema.h"

#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  // parse arguments
  struct arguments arguments = { NULL, 0 };
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  // load schema
  struct schema_t schema;
  schema_init(&schema);

  

  return 0;
}
