#ifndef ARGPARSE_H
#define ARGPARSE_H

#include "main.h"

#include <argp.h>

struct arguments_t
{
  const char *schemafile;
  int verbosity;
  unsigned int scale;
};

extern struct arguments_t arguments;

int argparse (int argc, char *argv[]);

#endif
