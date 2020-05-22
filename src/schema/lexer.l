
%{

#include "schema/schema.h"

#include "schema/parsertypes.h"
#include "schema/parser.h"

#include <errno.h>
#include <string.h>
#include <stdarg.h>

static int
strapp (char **buf, size_t *len, const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  int addlen = vsnprintf(NULL, 0, format, ap);
  va_end(ap);

  if (addlen < 0)
    return 1;

  char *newstr = realloc(*buf, sizeof(**buf) * (*len + addlen + 1));
  if (!newstr)
    return 2;

  va_start(ap, format);
  vsnprintf(newstr + *len, addlen + 1, format, ap);
  va_end(ap);

  *buf = newstr;
  *len += addlen;

  return 0;
}

extern int yyerror (struct schema_t*, const char*);

char *string_buf = NULL;
size_t string_len = 0;

static int yy_top_state(void) __attribute__((unused));

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
                string_buf = NULL;
                string_len = 0;
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
                free(string_buf);
                string_buf = NULL;
                string_len = 0;
                exit(2);
                }
\\[0-7]{1,3}    {
                int oct;
                  scanf(yytext + 1, "%o", &oct);

                if (oct > 0xff)
                  {
                    yyerror(NULL, "octal escape out of bounds");
                    free(string_buf);
                    string_buf = NULL;
                    string_len = 0;
                    exit(2);
                  }

                int res = strapp(&string_buf, &string_len, "%i", oct);
                if (res)
                  {
                    yyerror(NULL, strerror(errno));
                    free(string_buf);
                    string_buf = NULL;
                    string_len = 0;
                    exit(2);
                  }
                }
\\[0-9]+        {
                yyerror(NULL, "bad escape sequence");
                free(string_buf);
                string_buf = NULL;
                string_len = 0;
                exit(2);
                }
\\n             {
                int res = strapp(&string_buf, &string_len, "\n");
                if (res)
                  {
                    yyerror(NULL, strerror(errno));
                    free(string_buf);
                    string_buf = NULL;
                    string_len = 0;
                    exit(2);
                  }
                }
\\t             {
                int res = strapp(&string_buf, &string_len, "\t");
                if (res)
                  {
                    yyerror(NULL, strerror(errno));
                    free(string_buf);
                    string_buf = NULL;
                    string_len = 0;
                    exit(2);
                  }
                }
\\r             {
                int res = strapp(&string_buf, &string_len, "\r");
                if (res)
                  {
                    yyerror(NULL, strerror(errno));
                    free(string_buf);
                    string_buf = NULL;
                    string_len = 0;
                    exit(2);
                  }
                }
\\b             {
                int res = strapp(&string_buf, &string_len, "\b");
                if (res)
                  {
                    yyerror(NULL, strerror(errno));
                    free(string_buf);
                    string_buf = NULL;
                    string_len = 0;
                    exit(2);
                  }
                }
\\f             {
                int res = strapp(&string_buf, &string_len, "\f");
                if (res)
                  {
                    yyerror(NULL, strerror(errno));
                    free(string_buf);
                    string_buf = NULL;
                    string_len = 0;
                    exit(2);
                  }
                }
\\(.|\n)        {
                int res = strapp(&string_buf, &string_len, "%c", yytext[1]);
                if (res)
                  {
                    yyerror(NULL, strerror(errno));
                    free(string_buf);
                    string_buf = NULL;
                    string_len = 0;
                    exit(2);
                  }
                }
[^\\\n\"]+      {
                int res = strapp(&string_buf, &string_len, "%s", yytext);
                if (res)
                  {
                    yyerror(NULL, strerror(errno));
                    free(string_buf);
                    string_buf = NULL;
                    string_len = 0;
                    exit(2);
                  }
                }
}

[a-zA-Z_][0-9a-zA-Z_]* {
                char *string = strdup(yytext);
                if (!str)
                  {
                    yyerror(NULL, strerror(errno));
                    exit(1);
                  }
                yylval.string = string;
                return IDENTIFIER;
                }

0[0-7]*|0[xX][0-9a-fA-F]+|[1-9][0-9]* {
                int errsv = errno;
                errno = 0;
                unsigned long num = strtoul(yytext, NULL, 0);
                if (errno)
                  {
                    yyerror(NULL, strerror(errno));
                    exit(1);
                  }
                yylval.number = num;
                errno = errsv;
                return NUMBER;
                }

[ \t]
\n              return NEWLINE;

.               return *yytext;

%%
