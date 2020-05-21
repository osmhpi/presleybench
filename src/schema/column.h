#ifndef COLUMN_H
#define COLUMN_H

#include "schema/datatype.h"
#include "schema/reference.h"
#include "schema/stringlist.h"

struct column_t
{
  char *name;
  struct datatype_t type;
  int primary_key;
  size_t pool;
  struct reference_t poolref;
  struct stringlist_t poolarr;
};

void column_init (struct column_t*);

void column_fini (struct column_t*);

#endif
