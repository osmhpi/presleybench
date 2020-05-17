
%{

#include "schema.h"

#include <stdio.h>
#include <stdlib.h>

extern const char *yyfilename;
extern const char *program_invocation_short_name;
extern int yylineno;

void
yyerror (__attribute__((unused)) struct schema_t *schema, char const *s)
{
  fprintf(stderr, "%s: %s:%d: %s\n", program_invocation_short_name, yyfilename, yylineno, s);
}

extern int yylex(void);

%}

%parse-param { struct schema_t *schema }

%token TABLE COLUMN NAME TYPE PRIMARYKEY
%token SMALLINT REAL FLOAT CHAR
%token STRING NUMBER
%token NEWLINE

%start schema

%%

schema              : vertical_space table_list vertical_space
                    | vertical_space
                    ;

table_list          : table_list property_delimiter table
                    |                               table
                    ;

table               : TABLE vertical_space '{' table_definition '}'
                    ;

table_definition    : vertical_space table_parts vertical_space
                    | vertical_space
                    ;

table_parts         : table_parts property_delimiter table_part
                    |                                table_part
                    ;

table_part          : name_property
                    | column
                    ;

name_property       : NAME '=' STRING
                    ;

column              : COLUMN vertical_space'{' column_definition '}'
                    ;

column_definition   : vertical_space column_parts vertical_space
                    | vertical_space
                    ;

column_parts        : column_parts property_delimiter column_part
                    |                                 column_part
                    ;

column_part         : name_property
                    | type_property
                    | PRIMARYKEY
                    ;

type_property       : TYPE '=' datatype
                    ;

datatype            : simple_datatype
                    | complex_datatype
                    ;

simple_datatype     : REAL
                    | FLOAT
                    | SMALLINT
                    ;

complex_datatype    : complex_typename '(' NUMBER ')'
                    ;

complex_typename    : CHAR
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
