#ifndef PARSERTYPES_H
#define PARSERTYPES_H

#include "util/list.h"
#include "util/assert.h"

#include "schema/datatype.h"
#include "schema/column.h"
#include "schema/table.h"
#include "schema/foreignkey.h"

struct list_string_t
{
  size_t n;
  char **str;
};

enum reference_e
{
  REFERENCE_NONE,
  REFERENCE_TABLE,
  REFERENCE_COLUMN,
};

struct reference_t
{
  enum reference_e type;
  void *ref;
  char *string;
};

enum property_e
{
  PROPERTY_NAME,
  PROPERTY_ROWS,
  PROPERTY_TYPE,
  PROPERTY_PRIMARYKEY,
  PROPERTY_POOL,
  PROPERTY_POOLREF,
  PROPERTY_POOLARR,
  PROPERTY_FOREIGNKEY,
  PROPERTY_COLUMN,
  PROPERTY_TABLE,
};

struct property_t
{
  enum property_e type;
  union {
    char *string;
    long int number;
    struct datatype_t datatype;
    struct column_t column;
    struct table_t table;
    struct reference_t reference;
    struct foreignkey_t foreignkey;
    struct list_t list;
  };
};

const char *strproperty (enum property_e) att_warn_unused_result;

int column_add_property (struct column_t*, struct property_t) att_warn_unused_result;
int table_add_property (struct table_t*, struct property_t) att_warn_unused_result;
int schema_add_property (struct schema_t*, struct property_t) att_warn_unused_result;

#endif
