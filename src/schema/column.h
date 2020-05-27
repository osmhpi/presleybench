#ifndef COLUMN_H
#define COLUMN_H

#include "util/assert.h"
#include "schema/datatype.h"
#include "schema/valuepool.h"

struct table_t;

struct column_t
{
  char *name;
  struct table_t *parent;

  struct datatype_t type;
  struct valuepool_t pool;

  int primary_key;
};

int column_init (struct column_t*) att_warn_unused_result;

void column_fini (struct column_t*);

#endif
