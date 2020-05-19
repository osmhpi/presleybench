
%{

#include "schema/property.h"
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
      case PROPERTY_COLUMN:
        {
          struct column_t *column = malloc(sizeof(*column));
          if (!column)
            {
              yyerror(NULL, "%s", strerror(errno));
              return 2;
            }
          column_init(column);

          *column = property.column;

          int res = table_column_add(table, column);
          if (res)
            {
              yyerror(NULL, "%s", strerror(errno));
              return res;
            }

          break;
        }
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
  enum datatype_e typename;
  struct datatype_t datatype;
  struct column_t column;
  struct table_t table;
  struct property_t property;
}

%parse-param { struct schema_t *schema }

%token TABLE COLUMN NAME TYPE PRIMARYKEY
%token INT SMALLINT NUMERIC REAL FLOAT CHAR DATETIME
%token<string> STRING
%token<number> NUMBER
%token NEWLINE

%type<table> table table_definition table_parts
%type<column> column column_definition column_parts
%type<datatype> datatype length_datatype
%type<typename> simple_typename length_typename
%type<property> column_part table_part schema_part property

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
                    { column_init(&$$); }
                    ;

column_parts        : column_parts property_delimiter column_part
                    {
                      $$ = $1;
                      int res = column_add_property(&$$, $3);
                      if (res)
                        exit(1);
                    }
                    |                                 column_part
                    {
                      column_init(&$$);
                      int res = column_add_property(&$$, $1);
                      if (res)
                        exit(1);
                    }
                    ;

column_part         : property
                    { $$ = $1; }
                    ;

property            : NAME '=' STRING
                    { $$.type = PROPERTY_NAME; $$.string = $3; }
                    | TYPE '=' datatype
                    { $$.type = PROPERTY_TYPE; $$.datatype = $3; }
                    | PRIMARYKEY
                    { $$.type = PROPERTY_PRIMARYKEY; }
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
