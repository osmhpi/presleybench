
%{

#include "schema/parsertypes.h"
#include "argparse.h"
#include "schema/foreignkey.h"
#include "schema/schema.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

extern const char *yyfilename;
extern int yylineno;
extern const char *program_invocation_name;

void
yyerror (__attribute__((unused)) struct schema_t *schema, char const *format, ...)
{
  fprintf(stderr, "%s: %s:%d: ", program_invocation_name, yyfilename, yylineno);

  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);

  fprintf(stderr, "\n");
}

extern int yylex(void);

static int
column_add_property (struct column_t *column, struct property_t property)
{
  switch (property.type)
    {
      case PROPERTY_NAME:
        column->name = property.string;
        break;
      case PROPERTY_TYPE:
        column->type = property.datatype;
        break;
      case PROPERTY_PRIMARYKEY:
        column->primary_key = 1;
        break;
      case PROPERTY_POOL:
        {
          int res = valuepool_init(&column->pool, VALUEPOOL_CAPACITY);
          if (res)
            return res;

          column->pool.capacity = property.number;
          break;
        }
      case PROPERTY_POOLREF:
        {
          if (property.reference.type != REFERENCE_COLUMN)
            {
              yyerror(NULL, "%s:%s: pool reference to something not a column", "<table>", column->name);
              return 1;
            }

          struct column_t *c = property.reference.ref;

          switch (c->pool.type)
            {
              case VALUEPOOL_CAPACITY:
              case VALUEPOOL_STRINGS:
                break;
              case VALUEPOOL_REFERENCE:
                yyerror(NULL, "%s:%s: pool reference to another pool reference", "<table>", column->name);
                return 1;
              default:
                yyerror(NULL, "%s:%s: pool reference to uninitialized pool", "<table>", column->name);
                return 1;
            }

          int res = valuepool_init(&column->pool, VALUEPOOL_REFERENCE);
          if (res)
            return res;

          column->pool.ref = &c->pool;
          break;
        }
      case PROPERTY_POOLARR:
        {
          int res = valuepool_init(&column->pool, VALUEPOOL_STRINGS);
          if (res)
            return res;

          column->pool._strings = property.list;
          break;
        }
      default:
        yyerror(NULL, "unrecognized poperty '%s' at column scope", strproperty(property.type));
        return 1;
        break;
    }

  return 0;
}

static int
table_add_property (struct table_t *table, struct property_t property)
{
  switch (property.type)
    {
      case PROPERTY_NAME:
        table->name = property.string;
        break;
      case PROPERTY_ROWS:
        table->rows = property.number;
        break;
      case PROPERTY_COLUMN:
        {
          struct column_t *column = malloc(sizeof(*column));
          if (!column)
            {
              yyerror(NULL, "%s", strerror(errno));
              return 2;
            }

          int res = column_init(column);
          if (res)
            {
              yyerror(NULL, "%s", strerror(errno));
              free(column);
              return res;
            }

          *column = property.column;

          res = table_column_add(table, column);
          if (res)
            {
              yyerror(NULL, "%s", strerror(errno));
              free(column);
              return res;
            }

          break;
        }
      case PROPERTY_FOREIGNKEY:
        {
          struct foreignkey_t *fk = malloc(sizeof(*fk));
          if (!fk)
            {
              yyerror(NULL, "%s", strerror(errno));
              return 2;
            }
          foreignkey_init(fk);

          *fk = property.foreignkey;

          int res = table_foreignkey_add(table, fk);
          if (res)
            {
              yyerror(NULL, "%s", strerror(errno));
              return res;
            }

          break;
        }
        break;
      default:
        yyerror(NULL, "unrecognized poperty '%s' at table scope", strproperty(property.type));
        return 1;
        break;
    }

  return 0;
}

static int
schema_add_property (struct schema_t *schema, struct property_t property)
{
  switch (property.type)
    {
      case PROPERTY_TABLE:
        {
          struct table_t *table = malloc(sizeof(*table));
          if (!table)
            {
              yyerror(NULL, "%s", strerror(errno));
              return 2;
            }
          table_init(table);

          *table = property.table;

          int res = schema_table_add(schema, table);
          if (res)
            {
              yyerror(NULL, "%s", strerror(errno));
              return res;
            }

          break;
        }
      default:
        yyerror(NULL, "unrecognized property '%s' at schema scope", strproperty(property.type));
        return 1;
        break;
    }

  return 0;
}

%}

%union {
  char *string;
  int number;
  struct list_t list;
  enum datatype_e typename;
  struct datatype_t datatype;
  struct column_t column;
  struct table_t table;
  struct property_t property;
  struct reference_t reference;
  struct foreignkey_t foreignkey;
}

%parse-param { struct schema_t *schema }

%token TABLE COLUMN NAME ROWS TYPE PRIMARYKEY FOREIGNKEY POOL
%token INT SMALLINT NUMERIC REAL FLOAT CHAR VARCHAR DATETIME
%token<string> STRING IDENTIFIER
%token<number> NUMBER
%token REFARROW
%token NEWLINE

%left '+' '-'
%left '*' '/'

%type<table> table table_definition table_parts
%type<column> column column_definition column_parts
%type<datatype> datatype length_datatype
%type<typename> simple_typename length_typename
%type<property> column_part table_part schema_part property
%type<number> expression
%type<reference> reference qualified_name
%type<list> column_list string_list string_array

%start schema

%%

schema              : vertical_space schema_parts vertical_space
                    | vertical_space
                    ;

schema_parts        : schema_parts property_delimiter schema_part
                    {
                      int res = schema_add_property(schema, $3);
                      if (res)
                        exit(1);
                    }
                    |                                 schema_part
                    {
                      int res = schema_add_property(schema, $1);
                      if (res)
                        exit(1);
                    }
                    ;

schema_part         : property
                    { $$ = $1; }
                    | table
                    { $$.type = PROPERTY_TABLE; $$.table = $1; }
                    ;

table               : TABLE vertical_space '{' table_definition '}'
                    { $$ = $4; }
                    ;

table_definition    : vertical_space table_parts vertical_space
                    { $$ = $2; }
                    | vertical_space
                    { table_init(&$$); }
                    ;

table_parts         : table_parts property_delimiter table_part
                    {
                      $$ = $1;
                      int res = table_add_property(&$$, $3);
                      if (res)
                        exit(1);
                    }
                    |                                table_part
                    {
                      table_init(&$$);
                      int res = table_add_property(&$$, $1);
                      if (res)
                        exit(1);
                    }
                    ;

table_part          : property
                    { $$ = $1; }
                    | column
                    { $$.type = PROPERTY_COLUMN; $$.column = $1; }
                    ;

column              : COLUMN vertical_space'{' column_definition '}'
                    { $$ = $4; }
                    ;

column_definition   : vertical_space column_parts vertical_space
                    { $$ = $2; }
                    | vertical_space
                    {
                      int res = column_init(&$$);
                      if (res)
                        {
                          yyerror(NULL, "%s", strerror(errno));
                          exit(res);
                        }
                    }
                    ;

column_parts        : column_parts property_delimiter column_part
                    {
                      $$ = $1;
                      int res = column_add_property(&$$, $3);
                      if (res)
                        exit(res);
                    }
                    |                                 column_part
                    {
                      int res = column_init(&$$);
                      if (res)
                        {
                          yyerror(NULL, "%s", strerror(errno));
                          exit(res);
                        }

                      res = column_add_property(&$$, $1);
                      if (res)
                        exit(res);
                    }
                    ;

column_part         : property
                    { $$ = $1; }
                    ;

property            : NAME '=' STRING
                    { $$.type = PROPERTY_NAME; $$.string = $3; }
                    | ROWS '=' expression
                    { $$.type = PROPERTY_ROWS; $$.number = $3; }
                    | TYPE '=' datatype
                    { $$.type = PROPERTY_TYPE; $$.datatype = $3; }
                    | PRIMARYKEY
                    { $$.type = PROPERTY_PRIMARYKEY; }
                    | POOL '=' expression
                    { $$.type = PROPERTY_POOL; $$.number = $3; }
                    | POOL '=' reference
                    { $$.type = PROPERTY_POOLREF; $$.reference = $3; }
                    | POOL '=' string_array
                    { $$.type = PROPERTY_POOLARR; $$.list = $3; }
                    | FOREIGNKEY column_list REFARROW qualified_name '(' column_list ')'
                    {
                      $$.type = PROPERTY_FOREIGNKEY;
                      foreignkey_init(&$$.foreignkey);

                      if ($4.type != REFERENCE_TABLE)
                        {
                          yyerror(NULL, "foreign key reference to something not a table");
                          exit(1);
                        }
                      $$.foreignkey.rhs_table = $4.ref;

                      $$.foreignkey._lhs = $2;
                      $$.foreignkey._rhs = $6;
                    }
                    ;

string_array        : '[' string_list ']'
                    { $$ = $2; }
                    | '[' /* empty */ ']'
                    { list_init(&$$); }
                    ;

string_list         : string_list ',' STRING
                    {
                      $$ = $1;
                      int res = list_add(&$$, $3);
                      if (res)
                        {
                          yyerror(NULL, "%s", strerror(errno));
                          exit(res);
                        }
                    }
                    | STRING
                    {
                      list_init(&$$);
                      int res = list_add(&$$, $1);
                      if (res)
                        {
                          yyerror(NULL, "%s", strerror(errno));
                          exit(res);
                        }
                    }
                    ;

column_list         : IDENTIFIER
                    {
                      list_init(&$$);
                      int res = list_add(&$$, $1);
                      if (res)
                        {
                          yyerror(NULL, "%s", strerror(errno));
                          exit(res);
                        }
                    }
                    | column_list ',' IDENTIFIER
                    {
                      $$ = $1;
                      int res = list_add(&$$, $3);
                      if (res)
                        {
                          yyerror(NULL, "%s", strerror(errno));
                          exit(res);
                        }
                    }
                    ;

expression          : expression '+' expression
                    { $$ = $1 + $3; }
                    | expression '-' expression
                    { $$ = $1 - $3; }
                    | expression '*' expression
                    { $$ = $1 * $3; }
                    | expression '/' expression
                    { $$ = $1 / $3; }
                    | '(' expression ')'
                    { $$ = $2; }
                    | NUMBER
                    { $$ = $1; }
                    | IDENTIFIER
                    {
                      if (!strcmp($1, "S"))
                        $$ = arguments.scale;
                      else
                        {
                          yyerror(NULL, "unrecognized identifier '%s'", $1);
                          exit(1);
                        }

                      free($1);
                    }
                    ;

reference           : '&' qualified_name
                    { $$ = $2; }
                    ;

qualified_name      : IDENTIFIER
                    {
                      $$.ref = NULL;

                      size_t i;
                      for (i = 0; i < schema->tables.n; ++i)
                        {
                          if (!strcmp($1, schema->tables.tables[i]->name))
                            {
                              $$.type = REFERENCE_TABLE;
                              $$.ref = schema->tables.tables[i];
                            }
                        }

                      if (!$$.ref)
                        {
                          yyerror(NULL, "referenced table '%s' not found in schema", $1);
                          exit(1);
                        }

                      free($1);
                    }
                    | qualified_name '.' IDENTIFIER
                    {
                      $$.ref = NULL;

                      size_t i;
                      switch ($1.type)
                        {
                          case REFERENCE_TABLE:
                            for (i = 0; i < ((struct table_t*)$1.ref)->columns.n; ++i)
                              {
                                if (!strcmp($3, ((struct table_t*)$1.ref)->columns.columns[i]->name))
                                  {
                                    $$.type = REFERENCE_COLUMN;
                                    $$.ref = ((struct table_t*)$1.ref)->columns.columns[i];
                                  }
                              }

                            if (!$$.ref)
                              {
                                yyerror(NULL, "referenced column '%s' not found in table '%s'", $3, ((struct table_t*)$1.ref)->name);
                                exit(1);
                              }

                            break;
                          default:
                            yyerror(NULL, "unable to resolve reference '%s' in something not a schema or a table", $3);
                            exit(1);
                        }

                      free($3);
                    }
                    ;

datatype            : simple_typename
                    { $$.name = $1; $$.length = 0; }
                    | length_datatype
                    { $$ = $1; }
                    ;

simple_typename     : INT
                    { $$ = DATATYPE_INT; }
                    | SMALLINT
                    { $$ = DATATYPE_SMALLINT; }
                    | REAL
                    { $$ = DATATYPE_REAL; }
                    | FLOAT
                    { $$ = DATATYPE_FLOAT; }
                    | DATETIME
                    { $$ = DATATYPE_DATETIME; }
                    ;

length_datatype     : length_typename '(' NUMBER ')'
                    { $$.name = $1; $$.length = $3; }
                    ;

length_typename     : CHAR
                    { $$ = DATATYPE_CHAR; }
                    | VARCHAR
                    { $$ = DATATYPE_VARCHAR; }
                    | NUMERIC
                    { $$ = DATATYPE_NUMERIC; }
                    ;

property_delimiter  : vertical_space ';' vertical_space
                    | newlines
                    ;

vertical_space      : newlines
                    | /* empty */
                    ;

newlines            : newlines NEWLINE
                    |          NEWLINE
                    ;
