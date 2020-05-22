#ifndef COLUMN_H
#define COLUMN_H

#include "schema/datatype.h"
#include "schema/valuepool.h"

struct column_t
{
  char *name;
  struct datatype_t type;
  int primary_key;
  struct valuepool_t pool;
};

int column_init (struct column_t*);

void column_fini (struct column_t*);

#endif
