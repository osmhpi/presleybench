#ifndef ARGPARSE_H
#define ARGPARSE_H

#include "main.h"

#include "util/assert.h"

#include <argp.h>

struct arguments_t
{
  size_t rows;
  double sparsity;
  size_t threads;

  int verbosity;
};

extern struct arguments_t arguments;

int argparse (int argc, char *argv[]) att_warn_unused_result;

#endif
