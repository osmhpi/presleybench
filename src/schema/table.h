#ifndef TABLE_H
#define TABLE_H

#include "column.h"

#include <stddef.h>

struct table_t
{
  char *name;

  struct column_t **columns;
  size_t ncolumns;
};

void table_init (struct table_t*);

void table_fini (struct table_t*);

int table_column_add (struct table_t*, struct column_t *);

#endif
