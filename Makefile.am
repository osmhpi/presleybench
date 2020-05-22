
## process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

AM_YFLAGS = -d -v
AM_CPPFLAGS = -I$(top_srcdir)/src/

bin_PROGRAMS = presleybench

presleybench_SOURCES = \
  src/main.c src/main.h \
  src/argparse.c src/argparse.h \
  src/util/list.c src/util/list.h \
  src/schema/lexer.l src/schema/parser.y \
  src/schema/parsertypes.c src/schema/parsertypes.h \
  src/schema/schema.c src/schema/schema.h \
  src/schema/table.c src/schema/table.h \
  src/schema/column.c src/schema/column.h \
  src/schema/datatype.c src/schema/datatype.h \
  src/schema/foreignkey.c src/schema/foreignkey.h \
  src/schema/valuepool.c src/schema/valuepool.h \
  src/schema/validate.c src/schema/validate.h \
  src/schema/tpcc.c src/schema/tpcc.h \
  src/data/generate.c src/data/generate.h

presleybench_LDADD = $(argp_LIBS)

BUILT_SOURCES = \
  src/schema/parser.h
