#ifndef TABLE_H
#define TABLE_H

#include "util/list.h"
#include "schema/column.h"
#include "schema/foreignkey.h"

#include <stddef.h>

struct schema_t;

struct table_t
{
  char *name;
  struct schema_t *parent;

  size_t rows;

  union {
    struct list_t _columns;
    struct {
      size_t n;
      struct column_t **columns;
    } columns;
  };

  union {
    struct list_t _fks;
    struct list_fks_t {
      size_t n;
      struct foreignkey_t **fks;
    } fks;
  };
};

void table_init (struct table_t*);

void table_fini (struct table_t*);

int table_column_add (struct table_t*, struct column_t*);

int table_foreignkey_add (struct table_t*, struct foreignkey_t*);

struct column_t* table_get_column_by_name (struct table_t*, const char*);

#endif
