
## process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -I$(top_srcdir)/src/

bin_PROGRAMS = simplebench

simplebench_SOURCES = \
  src/simple/main.c src/simple/main.h \
  src/simple/argparse.c src/simple/argparse.h \
  src/simple/threads.c src/simple/threads.h \
  src/simple/data.c src/simple/data.h \
  src/simple/topology.c src/simple/topology.h \
  src/simple/index/index.c src/simple/index/index.h \
  src/simple/index/bplustree.c src/simple/index/bplustree.h \
  src/simple/index/groupkey.c src/simple/index/groupkey.h \
  src/util/assert.c src/util/assert.h \
  src/util/progress.c src/util/progress.h

simplebench_CFLAGS = $(pthread_CFLAGS)

simplebench_LDADD = -ldl $(argp_LIBS) $(pthread_LIBS) $(numa_LIBS)
