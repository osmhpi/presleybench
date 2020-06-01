#ifndef ARGPARSE_H
#define ARGPARSE_H

#include "main.h"

#include "util/assert.h"

#include <argp.h>

enum pin_strategy_e
{
  PIN_STRATEGY_CPU,
  PIN_STRATEGY_NODE
};

struct arguments_t
{
  size_t rows;
  double sparsity;

  int primary_node;
  const char *_primary_node;
  int replicate;
  int tree_search;

  enum pin_strategy_e pin_strategy;
  const char *_pin_strategy;

  int verbosity;
};

extern struct arguments_t arguments;

int argparse (int argc, char *argv[]) att_warn_unused_result;

#endif
