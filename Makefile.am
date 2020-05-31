
## process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

AM_YFLAGS = -d -v
AM_CPPFLAGS = -I$(top_srcdir)/src/

bin_PROGRAMS = presleybench simplebench

presleybench_SOURCES = \
  src/presley/main.c src/presley/main.h \
  src/presley/argparse.c src/presley/argparse.h \
  src/util/list.c src/util/list.h \
  src/util/assert.c src/util/assert.h \
  src/util/string.c src/util/string.h \
  src/schema/lexer.l src/schema/parser.y \
  src/schema/parsertypes.c src/schema/parsertypes.h \
  src/schema/schema.c src/schema/schema.h \
  src/schema/table.c src/schema/table.h \
  src/schema/column.c src/schema/column.h \
  src/schema/datatype.c src/schema/datatype.h \
  src/schema/foreignkey.c src/schema/foreignkey.h \
  src/schema/valuepool.c src/schema/valuepool.h \
  src/schema/validate.c src/schema/validate.h \
  src/schema/dump.c src/schema/dump.h \
  src/schema/tpcc.c src/schema/tpcc.h \
  src/data/generate.c src/data/generate.h \
  src/data/generators.c src/data/generators.h

presleybench_LDADD = $(argp_LIBS)

simplebench_SOURCES = \
  src/simple/main.c src/simple/main.h \
  src/simple/argparse.c src/simple/argparse.h \
  src/simple/bplustree.c src/simple/bplustree.h \
  src/simple/threads.c src/simple/threads.h \
  src/simple/data.c src/simple/data.h \
  src/util/assert.c src/util/assert.h

simplebench_CFLAGS = $(pthread_CFLAGS)

simplebench_LDADD = $(argp_LIBS) $(pthread_LIBS)

BUILT_SOURCES = \
  src/schema/parser.h
