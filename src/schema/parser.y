
%{

#include "util/assert.h"
#include "util/string.h"
#include "schema/parsertypes.h"
#include "presley/argparse.h"
#include "schema/foreignkey.h"
#include "schema/schema.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

extern const char *yyfilename;
extern int yylineno;

void
yyerror (att_unused struct schema_t *schema, char const *format, ...)
{
  int errnum = errno;

  fprintf(stderr, "%s:%s:%u: error: ", program_invocation_name, yyfilename, yylineno);

  if (format)
    {
      va_list args;
      va_start(args, format);
      vfprintf(stderr, format, args);
      va_end(args);
    }

  if(errnum || !format)
    fprintf(stderr, ": %s", strerror(errnum));

  fprintf(stderr, "\n");

  errno = errnum;
}

void
yywarning (const char *format, ...)
{
  int errnum = errno;

  fprintf(stderr, "%s:%s:%u: warning: ", program_invocation_name, yyfilename, yylineno);

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  if(errnum)
    fprintf(stderr, ": %s", strerror(errnum));

  fprintf(stderr, "\n");

  errno = errnum;
}

extern int yylex(void);

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
                      guard (0 == schema_add_property(schema, $3)) else { YYABORT; }
                    }
                    |                                 schema_part
                    {
                      guard (0 == schema_add_property(schema, $1)) else { YYABORT; }
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
                      guard (0 == table_add_property(&$$, $3)) else { YYABORT; }
                    }
                    |                                table_part
                    {
                      table_init(&$$);
                      guard (0 == table_add_property(&$$, $1)) else { YYABORT; }
                    }
                    ;

table_part          : property
                    { $$ = $1; }
                    | column
                    { $$.type = PROPERTY_COLUMN; $$.column = $1; }
                    ;

column              : COLUMN vertical_space '{' column_definition '}'
                    { $$ = $4; }
                    ;

column_definition   : vertical_space column_parts vertical_space
                    { $$ = $2; }
                    | vertical_space
                    {
                      guard (0 == column_init(&$$)) else { yyerror(NULL, NULL); YYABORT; }
                    }
                    ;

column_parts        : column_parts property_delimiter column_part
                    {
                      $$ = $1;
                      guard (0 == column_add_property(&$$, $3)) else { YYABORT; }
                    }
                    |                                 column_part
                    {
                      guard (0 == column_init(&$$)) else { yyerror(NULL, NULL); YYABORT; }
                      guard (0 == column_add_property(&$$, $1)) else { YYABORT; }
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
                          yyerror(NULL, "foreign key reference to something not a table: '%s'", $4.string);
                          free($4.string);
                          YYABORT;
                        }

                      $$.foreignkey.rhs_table = $4.ref;

                      $$.foreignkey._lhs = $2;
                      $$.foreignkey._rhs = $6;

                      free($4.string);
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
                      guard (0 == list_add(&$$, $3)) else { yyerror(NULL, NULL); free($3); YYABORT; }
                    }
                    | STRING
                    {
                      list_init(&$$);
                      guard (0 == list_add(&$$, $1)) else { yyerror(NULL, NULL); free($1); YYABORT; }
                    }
                    ;

column_list         : IDENTIFIER
                    {
                      list_init(&$$);
                      guard (0 == list_add(&$$, $1)) else { yyerror(NULL, NULL); free($1); YYABORT; }
                    }
                    | column_list ',' IDENTIFIER
                    {
                      $$ = $1;
                      guard (0 == list_add(&$$, $3)) else { yyerror(NULL, NULL); free($3); YYABORT; }
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
                        {
                          $$ = arguments.scale;
                        }
                      else
                        {
                          yyerror(NULL, "unrecognized identifier: '%s'", $1);
                          free($1);
                          YYABORT;
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
                      $$.string = $1;

                      guard (NULL != ($$.ref = schema_get_table_by_name(schema, $1))) else
                        {
                          yyerror(NULL, "unable to resolve table reference: '%s'", $1);
                          free($1);
                          YYABORT;
                        }

                      $$.type = REFERENCE_TABLE;
                    }
                    | qualified_name '.' IDENTIFIER
                    {
                      $$.ref = NULL;
                      $$.string = $1.string;

                      guard (0 <= astrcat(&$$.string, NULL, ".%s", $3)) else { yyerror(NULL, NULL); free($3); YYABORT; }

                      switch ($1.type)
                        {
                          case REFERENCE_TABLE:
                            guard (NULL != ($$.ref = table_get_column_by_name($1.ref, $3))) else
                              {
                                yyerror(NULL, "unable to resolve column reference: '%s'", $$.string);
                                free($3);
                                YYABORT;
                              }
                            $$.type = REFERENCE_COLUMN;
                            break;
                          default:
                            yyerror(NULL, "invalid reference to something not a table or a column: '%s'", $$.string);
                            free($3);
                            YYABORT;
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
