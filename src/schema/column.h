#ifndef COLUMN_H
#define COLUMN_H

#include "datatype.h"

struct column_t
{
  char *name;
  struct datatype_t type;
  int primary_key;
};

void column_init (struct column_t*);

void column_fini (struct column_t*);

#endif
