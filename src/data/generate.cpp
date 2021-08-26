
#include "data/generate.h"
#include "data/generators.h"

#include "util/assert.h"

int
generate_schema_data (struct schema_t *schema)
{
  generators_populate(schema);

  return 0;
}
