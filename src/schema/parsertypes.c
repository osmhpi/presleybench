
#include "schema/parsertypes.h"

#include "util/assert.h"
#include "schema/schema.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

extern void yyerror (struct schema_t*, const char*, ...);
extern void yywarning (const char*, ...);

#define WARNING_REDEFINITION "redefinition of property '%s'"

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

  debug_error("unexpected value for 'property': '%i'", property);
  return "<invalid>";
}

int
column_add_property (struct column_t *column, struct property_t property)
{
  switch (property.type)
    {
      case PROPERTY_NAME:
        if (column->name)
          {
            yywarning(WARNING_REDEFINITION, strproperty(property.type));
            free(column->name);
          }
        column->name = property.string;
        break;
      case PROPERTY_TYPE:
        if (column->type.name != DATATYPE_NONE)
          {
            yywarning(WARNING_REDEFINITION, strproperty(property.type));
          }
        column->type = property.datatype;
        break;
      case PROPERTY_PRIMARYKEY:
        if (column->primary_key)
          {
            yywarning(WARNING_REDEFINITION, strproperty(property.type));
          }
        column->primary_key = 1;
        break;
      case PROPERTY_POOL:
        {
          if (column->pool.type != VALUEPOOL_NONE)
            {
              yywarning(WARNING_REDEFINITION, strproperty(property.type));
              valuepool_fini(&column->pool);
            }

          int res;
          guard (0 == (res = valuepool_init(&column->pool, VALUEPOOL_CAPACITY))) else { return res; }

          column->pool.capacity = property.number;
          break;
        }
      case PROPERTY_POOLREF:
        {
          if (column->pool.type != VALUEPOOL_NONE)
            {
              yywarning(WARNING_REDEFINITION, strproperty(property.type));
              valuepool_fini(&column->pool);
            }

          if (property.reference.type != REFERENCE_COLUMN)
            {
              yyerror(NULL, "pool reference to something not a column: '%s'", property.reference.string);
              return 1;
            }

          struct column_t *c = property.reference.ref;
          while (c && c->pool.type == VALUEPOOL_REFERENCE)
            c = c->pool.ref->column;

          if (NULL == c)
            {
              yyerror(NULL, "unable to resolve pool reference: '%s'", property.reference.string);
              return 1;
            }

          switch (c->pool.type)
            {
              case VALUEPOOL_CAPACITY:
              case VALUEPOOL_STRINGS:
                break;
              default:
                yyerror(NULL, "invalid pool reference: '%s'", property.reference.string);
                return 1;
            }

          int res;
          guard (0 == (res = valuepool_init(&column->pool, VALUEPOOL_REFERENCE))) else { return res; }

          column->pool.ref = &c->pool;
          break;
        }
      case PROPERTY_POOLARR:
        {
          if (column->pool.type != VALUEPOOL_NONE)
            {
              yywarning(WARNING_REDEFINITION, strproperty(property.type));
              valuepool_fini(&column->pool);
            }

          int res;
          guard (0 == (res = valuepool_init(&column->pool, VALUEPOOL_STRINGS))) else { return res; }

          column->pool._strings = property.list;
          break;
        }
      default:
        yyerror(NULL, "unrecognized column poperty: '%s'", strproperty(property.type));
        return 1;
    }

  return 0;
}

int
table_add_property (struct table_t *table, struct property_t property)
{
  switch (property.type)
    {
      case PROPERTY_NAME:
        if (table->name)
          {
            yywarning(WARNING_REDEFINITION, strproperty(property.type));
            free(table->name);
          }
        table->name = property.string;
        break;
      case PROPERTY_ROWS:
        if (table->rows)
          {
            yywarning(WARNING_REDEFINITION, strproperty(property.type));
          }
        if (0 == property.number)
          {
            yywarning("table row count defined to zero");
          }
        table->rows = property.number;
        break;
      case PROPERTY_COLUMN:
        {
          struct column_t *column;
          guard (NULL != (column = malloc(sizeof(*column)))) else
            {
              yyerror(NULL, NULL);
              return 2;
            }

          int res;
          guard (0 == (res = column_init(column))) else
            {
              yyerror(NULL, NULL);
              free(column);
              return res;
            }

          *column = property.column;

          guard (0 == (res = table_column_add(table, column))) else
            {
              yyerror(NULL, NULL);
              column_fini(column);
              free(column);
              return res;
            }

          break;
        }
      case PROPERTY_FOREIGNKEY:
        {
          if (property.foreignkey.rhs.n != property.foreignkey.lhs.n)
            {
              yyerror(NULL, "foreign key size mismatch");
              return 1;
            }

          struct foreignkey_t *fk;
          guard (NULL != (fk = malloc(sizeof(*fk)))) else
            {
              yyerror(NULL, NULL);
              return 2;
            }

          foreignkey_init(fk);

          *fk = property.foreignkey;

          guard (NULL != (fk->lhs.columns = malloc(sizeof(*fk->lhs.columns) * fk->lhs.n))) else
            {
              yyerror(NULL, NULL);
              foreignkey_fini(fk);
              free(fk);
              return 2;
            }

          guard (NULL != (fk->rhs.columns = malloc(sizeof(*fk->rhs.columns) * fk->rhs.n))) else
            {
              yyerror(NULL, NULL);
              foreignkey_fini(fk);
              free(fk);
              return 2;
            }

          size_t i;
          for (i = 0; i < fk->lhs.n; ++i)
            {
              char *name = fk->lhs.names[i];
              struct column_t *c;

              guard (NULL != (c = table_get_column_by_name(table, name))) else
                {
                  yyerror(NULL, "unable to resolve column '%s'", name);
                  foreignkey_fini(fk);
                  free(fk);
                  return 1;
                }

              fk->lhs.columns[i] = c;
            }

          for (i = 0; i < fk->rhs.n; ++i)
            {
              char *name = fk->rhs.names[i];
              struct column_t *c;

              guard (NULL != (c = table_get_column_by_name(fk->rhs_table, name))) else
                {
                  yyerror(NULL, "unable to resolve column '%s.%s'", fk->rhs_table->name, name);
                  foreignkey_fini(fk);
                  free(fk);
                  return 1;
                }

              fk->rhs.columns[i] = c;
            }

          int res;
          guard (0 == (res =  table_foreignkey_add(table, fk))) else
            {
              yyerror(NULL, NULL);
              foreignkey_fini(fk);
              free(fk);
              return res;
            }

          break;
        }
      default:
        yyerror(NULL, "unrecognized table poperty: '%s'", strproperty(property.type));
        return 1;
    }

  return 0;
}

int
schema_add_property (struct schema_t *schema, struct property_t property)
{
  switch (property.type)
    {
      case PROPERTY_TABLE:
        {
          struct table_t *table;
          guard (NULL != (table = malloc(sizeof(*table)))) else
            {
              yyerror(NULL, NULL);
              return 2;
            }

          table_init(table);

          *table = property.table;

          int res;
          guard (0 == (res = schema_table_add(schema, table))) else
            {
              yyerror(NULL, NULL);
              return res;
            }

          break;
        }
      default:
        yyerror(NULL, "unrecognized schema property: '%s'", strproperty(property.type));
        return 1;
    }

  return 0;
}
