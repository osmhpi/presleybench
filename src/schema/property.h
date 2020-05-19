#ifndef PROPERTY_H
#define PROPERTY_H

#include "schema/table.h"

enum property_e
{
  PROPERTY_NAME,
  PROPERTY_TYPE,
  PROPERTY_PRIMARYKEY,
  PROPERTY_COLUMN,
  PROPERTY_TABLE,
};

struct property_t
{
  enum property_e type;
  union {
    char *string;
    struct datatype_t datatype;
    struct column_t column;
    struct table_t table;
  };
};

const char *strproperty (enum property_e);

#endif
