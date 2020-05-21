#ifndef PROPERTY_H
#define PROPERTY_H

#include "schema/table.h"
#include "schema/reference.h"
#include "schema/foreignkey.h"
#include "schema/stringlist.h"

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
    struct stringlist_t stringlist;
  };
};

const char *strproperty (enum property_e);

#endif
