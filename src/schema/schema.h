#ifndef SCHEMA_H
#define SCHEMA_H

#include "util/list.h"
#include "schema/table.h"

#include <stddef.h>

struct schema_t
{
  union {
    struct list_t _tables;
    struct {
      size_t n;
      struct table_t **tables;
    } tables;
  };
};

void schema_init (struct schema_t*);

void schema_fini (struct schema_t*);

int schema_table_add (struct schema_t*, struct table_t*);

int schema_parse_from_file (struct schema_t*, const char*);

#endif
