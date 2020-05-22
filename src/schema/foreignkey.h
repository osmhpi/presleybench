#ifndef FOREIGNKEY_H
#define FOREIGNKEY_H

#include "util/list.h"
#include "schema/column.h"

struct foreignkey_t
{
  int resolved;

  struct table_t *lhs_table;
  struct table_t *rhs_table;

  union {
    struct list_t _lhs;
    struct {
      size_t n;
      char **names;
      struct column_t *columns;
    } lhs;
  };

  union {
    struct list_t _rhs;
    struct {
      size_t n;
      char **names;
      struct column_t *columns;
    } rhs;
  };
};

void foreignkey_init (struct foreignkey_t*);

void foreignkey_fini (struct foreignkey_t*);

#endif
