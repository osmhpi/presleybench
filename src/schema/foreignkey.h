#ifndef FOREIGNKEY_H
#define FOREIGNKEY_H

#include "column.h"

struct columnlist_t
{
  size_t n;
  char **names;
  struct column_t **cols;
};

struct foreignkey_t
{
  struct table_t *lhs_table;
  struct table_t *rhs_table;

  struct columnlist_t lhs;
  struct columnlist_t rhs;
};

void foreignkey_init (struct foreignkey_t*);

void foreignkey_fini (struct foreignkey_t*);

void columnlist_init (struct columnlist_t*);

void columnlist_fini (struct columnlist_t*);

int columnlist_column_add (struct columnlist_t*, char*);

#endif
