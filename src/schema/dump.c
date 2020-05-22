
#include "schema/dump.h"

#include <stdio.h>

static void dump_table (struct table_t*, FILE*);
static void dump_column (struct column_t*, FILE*);
static void dump_foreignkey (struct foreignkey_t*, FILE*);
static void dump_valuepool (struct valuepool_t*, FILE*);

int
dump_schema (struct schema_t *schema)
{
  FILE *out = fopen(".schema", "w");
  if (!out)
    return 1;

  fprintf(out, "schema ntables: %zu\n", schema->tables.n);

  size_t i;
  for (i = 0; i < schema->tables.n; ++i)
    {
      struct table_t *table = schema->tables.tables[i];

      fprintf(out, "\ntable #%zu:\n", i);
      dump_table(table, out);
    }

  fclose(out);

  return 0;
}

void
dump_table (struct table_t *table, FILE *out)
{
  fprintf(out, "  name: '%s'\n", table->name);
  fprintf(out, "  rows: %zu\n", table->rows);

  fprintf(out, "\n  ncolumns: %zu\n", table->columns.n);

  size_t i;
  for (i = 0; i < table->columns.n; ++i)
    {
      struct column_t *column = table->columns.columns[i];

      fprintf(out, "\n  column #%zu:\n", i);
      dump_column(column, out);
    }

  fprintf(out, "\n  nfks: %zu\n", table->fks.n);

  for (i = 0; i < table->fks.n; ++i)
    {
      struct foreignkey_t *fk = table->fks.fks[i];

      fprintf(out, "\n  foreign key #%zu:\n", i);
      dump_foreignkey(fk, out);
    }
}

static const char*
strdatatype (enum datatype_e name)
{
  switch (name)
    {
      case DATATYPE_NONE:
        return "<none>";
      case DATATYPE_INT:
        return "int";
      case DATATYPE_SMALLINT:
        return "smallint";
      case DATATYPE_NUMERIC:
        return "numeric";
      case DATATYPE_REAL:
        return "real";
      case DATATYPE_FLOAT:
        return "float";
      case DATATYPE_CHAR:
        return "char";
      case DATATYPE_VARCHAR:
        return "varchar";
      case DATATYPE_DATETIME:
        return "datetime";
      default:
        return "<unknown>";
    }
}

static const char*
strbool (int b)
{
  return (b ? "yes" : "no");
}

static const char*
strpooltype (enum valuepool_e type)
{
  switch (type)
    {
      case VALUEPOOL_NONE:
        return "<none>";
      case VALUEPOOL_CAPACITY:
        return "capacity";
      case VALUEPOOL_REFERENCE:
        return "reference";
      case VALUEPOOL_STRINGS:
        return "strings";
      default:
        return "<unknown>";
    }
}

void
dump_column (struct column_t *column, FILE *out)
{
  fprintf(out, "    name: '%s'\n", column->name);
  fprintf(out, "    datatype: %s(%zu)\n", strdatatype(column->type.name), column->type.length);

  fprintf(out, "    pool: %s\n", strpooltype(column->pool.type));
  dump_valuepool(&column->pool, out);

  fprintf(out, "    primary key: %s\n", strbool(column->primary_key));
}

void
dump_valuepool (struct valuepool_t *valuepool, FILE *out)
{
  switch (valuepool->type)
    {
      case VALUEPOOL_CAPACITY:
        fprintf(out, "      n: %zu\n", valuepool->capacity);
        break;
      case VALUEPOOL_REFERENCE:
        fprintf(out, "      ref: %p (%s.%s)\n", valuepool->ref, valuepool->ref->column->parent->name, valuepool->ref->column->name);
        break;
      case VALUEPOOL_STRINGS:
        {
          fprintf(out, "      [ ");

          size_t i;
          for (i = 0; i < valuepool->strings.n; ++i)
            {
              if (i > 0)
                fprintf(out, ", ");
              fprintf(out, "\"%s\"", valuepool->strings.strings[i]);
            }

          fprintf(out, " ]\n");

          break;
        }
      default:
        break;
    }
}

void
dump_foreignkey (struct foreignkey_t *fk, FILE *out)
{
  size_t i;
  fprintf(out, "\n    lhs: %p (%s)\n", fk->lhs_table, fk->lhs_table->name);
  fprintf(out, "      names  : ");
  for (i = 0; i < fk->lhs.n; ++i)
    {
      if (i > 0)
        fprintf(out, ", ");
      fprintf(out, "%s", fk->lhs.names[i]);
    }
  fprintf(out, "\n");
  fprintf(out, "      columns: ");
  for (i = 0; i < fk->lhs.n; ++i)
    {
      if (i > 0)
        fprintf(out, ", ");
      fprintf(out, "%p (%s)", fk->lhs.columns[i], fk->lhs.columns[i]->name);
    }
  fprintf(out, "\n");
  fprintf(out, "    rhs: %p (%s)\n", fk->rhs_table, fk->rhs_table->name);
  fprintf(out, "      names  : ");
  for (i = 0; i < fk->rhs.n; ++i)
    {
      if (i > 0)
        fprintf(out, ", ");
      fprintf(out, "%s", fk->rhs.names[i]);
    }
  fprintf(out, "\n");
  fprintf(out, "      columns: ");
  for (i = 0; i < fk->rhs.n; ++i)
    {
      if (i > 0)
        fprintf(out, ", ");
      fprintf(out, "%p (%s)", fk->rhs.columns[i], fk->rhs.columns[i]->name);
    }
  fprintf(out, "\n");
}
