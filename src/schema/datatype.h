#ifndef DATATYPE_H
#define DATATYPE_H

#include <stddef.h>

enum datatype_e
{
  DATATYPE_NONE,
  DATATYPE_INT,
  DATATYPE_SMALLINT,
  DATATYPE_NUMERIC,
  DATATYPE_REAL,
  DATATYPE_FLOAT,
  DATATYPE_CHAR,
  DATATYPE_VARCHAR,
  DATATYPE_DATETIME,
};

struct datatype_t
{
  enum datatype_e name;
  size_t length;
};

#endif
