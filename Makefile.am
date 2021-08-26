
## process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -I$(top_srcdir)/src/

bin_PROGRAMS = simplebench

simplebench_SOURCES = \
  src/simple/main.cpp src/simple/main.hpp \
  src/simple/argparse.cpp src/simple/argparse.hpp \
  src/simple/threads.cpp src/simple/threads.hpp \
  src/simple/data.cpp src/simple/data.hpp \
  src/simple/index/index.cpp src/simple/index/index.hpp \
  src/simple/index/bplustree.cpp src/simple/index/bplustree.hpp \
  src/simple/index/groupkey.cpp src/simple/index/groupkey.hpp \
  src/util/assert.cpp src/util/assert.hpp \
  src/util/progress.cpp src/util/progress.hpp

simplebench_CXXFLAGS = -I$(top_srcdir)/pgasus/include -I$(top_srcdir)/pgasus/src

simplebench_LDADD = -ldl $(argp_LIBS) -L$(top_srcdir)/pgasus/bin/ -lPGASUS -lPGASUS_base -lPGASUS_msource
