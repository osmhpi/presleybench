
#include "property.h"

const char*
strproperty (enum property_e property)
{
  switch (property)
    {
      case PROPERTY_NAME:
        return "name";
      case PROPERTY_TYPE:
        return "type";
      case PROPERTY_PRIMARYKEY:
        return "primary key";
      case PROPERTY_COLUMN:
        return "column";
      case PROPERTY_TABLE:
        return "table";
    }

  return "<invalid>";
}

