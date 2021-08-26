#ifndef ARGPARSE_H
#define ARGPARSE_H

#include "main.hpp"

#include "util/assert.hpp"
#include "simple/index/index.hpp"

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

  int index_search;
  enum index_type_e index_type;
  const char *_index_type;

  enum pin_strategy_e pin_strategy;
  const char *_pin_strategy;

  int verify;

  int verbosity;
};

extern struct arguments_t arguments;

int argparse (int argc, char *argv[]) _warn_unused_result;

#endif
