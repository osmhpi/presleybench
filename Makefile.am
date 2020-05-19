
## process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

AM_YFLAGS = -d

bin_PROGRAMS = presleybench

presleybench_SOURCES = \
  src/main.c src/main.h \
  src/schema/schema.c src/schema/schema.h \
  src/schema/lexer.l src/schema/parser.y \
  src/schema/table.c src/schema/table.h \
  src/schema/column.c src/schema/column.h \
  src/schema/datatype.c src/schema/datatype.h \
  src/schema/property.c src/schema/property.h \
  src/schema/validate.c src/schema/validate.h \
  src/schema/tpcc.c src/schema/tpcc.h

presleybench_LDADD = $(argp_LIBS)

BUILT_SOURCES = \
  src/schema/parser.h
