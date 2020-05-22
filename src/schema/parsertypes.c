
#include "schema/parsertypes.h"

const char*
strproperty (enum property_e property)
{
  switch (property)
    {
      case PROPERTY_NAME:
        return "name";
      case PROPERTY_ROWS:
        return "rows";
      case PROPERTY_TYPE:
        return "type";
      case PROPERTY_PRIMARYKEY:
        return "primary key";
      case PROPERTY_POOL:
      case PROPERTY_POOLREF:
      case PROPERTY_POOLARR:
        return "pool";
      case PROPERTY_FOREIGNKEY:
        return "foreign key";
      case PROPERTY_COLUMN:
        return "column";
      case PROPERTY_TABLE:
        return "table";
    }

  return "<invalid>";
}

