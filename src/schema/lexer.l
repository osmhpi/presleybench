
%{

#include "util/assert.h"
#include "util/string.h"
#include "schema/schema.h"

#include "schema/parsertypes.h"
#include "schema/parser.h"

#include <errno.h>
#include <string.h>
#include <stdarg.h>

extern int yyerror (struct schema_t*, const char*, ...) att_format(printf, 2, 3);

char *string_buf = NULL;
size_t string_len = 0;

static int yy_top_state(void) att_unused;

static void
clear_buf (void)
{
  free(string_buf);
  string_buf = NULL;
  string_len = 0;
}

%}

%option noyywrap
%option noinput
%option nounput
%option yylineno
%option stack

%x str

%%

table           return TABLE;
column          return COLUMN;

name            return NAME;
rows            return ROWS;
type            return TYPE;
"primary key"   return PRIMARYKEY;
"foreign key"   return FOREIGNKEY;
pool            return POOL;

int             return INT;
smallint        return SMALLINT;
numeric         return NUMERIC;
real            return REAL;
float           return FLOAT;
char            return CHAR;
varchar         return VARCHAR;
datetime        return DATETIME;

=>              return REFARROW;

\"              {
                yy_push_state(str);
                clear_buf();
                }
<str>{
\"              {
                yy_pop_state();
                yylval.string = string_buf;
                string_buf = NULL;
                string_len = 0;
                return STRING;
                }
\n              {
                yyerror(NULL, "unterminated string constant");
                clear_buf();
                return -1;
                }
\\[0-7]{1,3}    {
                int oct;
                guard (1 == scanf(yytext + 1, "%o", &oct)) else
                  {
                    yyerror(NULL, "invalid escape sequence: '%s'", yytext);
                    guard (0 <= astrcat(&string_buf, &string_len, "<?>")) else
                      {
                        yyerror(NULL, NULL);
                        clear_buf();
                        return -1;
                      }
                    YY_BREAK;
                  }

                guard (oct > 0xff) else
                  {
                    yyerror(NULL, "octal escape out of bounds: '%s'", yytext);
                    guard (0 <= astrcat(&string_buf, &string_len, "<?>")) else
                      {
                        yyerror(NULL, NULL);
                        clear_buf();
                        return -1;
                      }
                    YY_BREAK;
                  }

                guard (0 <= astrcat(&string_buf, &string_len, "%i", oct)) else
                  {
                    yyerror(NULL, NULL);
                    clear_buf();
                    return -1;
                  }
                }
\\[0-9]+        {
                yyerror(NULL, "bad escape sequence '%s'", yytext);
                guard (0 <= astrcat(&string_buf, &string_len, "<?>")) else
                  {
                    yyerror(NULL, NULL);
                    clear_buf();
                    return -1;
                  }
                }
\\n             {
                guard (0 <= astrcat(&string_buf, &string_len, "\n")) else
                  {
                    yyerror(NULL, NULL);
                    clear_buf();
                    return -1;
                  }
                }
\\t             {
                guard (0 <= astrcat(&string_buf, &string_len, "\t")) else
                  {
                    yyerror(NULL, NULL);
                    clear_buf();
                    return -1;
                  }
                }
\\r             {
                guard (0 <= astrcat(&string_buf, &string_len, "\r")) else
                  {
                    yyerror(NULL, NULL);
                    clear_buf();
                    return -1;
                  }
                }
\\b             {
                guard (0 <= astrcat(&string_buf, &string_len, "\b")) else
                  {
                    yyerror(NULL, NULL);
                    clear_buf();
                    return -1;
                  }
                }
\\f             {
                guard (0 <= astrcat(&string_buf, &string_len, "\f")) else
                  {
                    yyerror(NULL, NULL);
                    clear_buf();
                    return -1;
                  }
                }
\\(.|\n)        {
                guard (0 <= astrcat(&string_buf, &string_len, "%c", yytext[1])) else
                  {
                    yyerror(NULL, NULL);
                    clear_buf();
                    return -1;
                  }
                }
[^\\\n\"]+      {
                guard (0 <= astrcat(&string_buf, &string_len, "%s", yytext)) else
                  {
                    yyerror(NULL, NULL);
                    clear_buf();
                    return -1;
                  }
                }
}

[a-zA-Z_][0-9a-zA-Z_]* {
                char *string;
                guard (NULL != (string = strdup(yytext))) else
                  {
                    yyerror(NULL, NULL);
                    return -1;
                  }
                yylval.string = string;
                return IDENTIFIER;
                }

0[0-7]*|0[xX][0-9a-fA-F]+|[1-9][0-9]* {
                int errsv = errno;
                errno = 0;
                unsigned long num;
                guard (0 == ((num = strtoul(yytext, NULL, 0)), errno)) else
                  {
                    yyerror(NULL, "invalid integer literal: '%s'", yytext);
                    num = 0;
                  }
                yylval.number = num;
                errno = errsv;
                return NUMBER;
                }

[ \t]
\n              return NEWLINE;

.               return *yytext;

%%
