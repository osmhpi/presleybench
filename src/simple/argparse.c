
#include "argparse.h"

#include "util/assert.h"
#include "schema/tpcc.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define DEFAULT_ROWS 100000000
#define DEFAULT_SPARSITY 2.0
#define DEFAULT_THREADS 4

const char *argp_program_version = PACKAGE_STRING;
const char *argp_program_bug_address = PACKAGE_BUGREPORT;

const char doc[] = PACKAGE_NAME " - a simple microbenchmark tool";
const char args_doc[] = "[--]";

static struct argp_option options[] =
{
  {"rows", 'r', "<number>", 0, "the amount of rows to populate", 0},
  {"sparsity", 's', "<fraction>", 0, "the sparsity of populated values. must be >= 1", 0},
  {"threads", 't', "<number>", 0, "the number of threads to spawn", 0},
  {"verbose", 'v', NULL, 0, "be more verbose", 0},
  {"quiet", 'q', NULL, 0, "be less verbose", 0},
  {NULL, 0, NULL, 0, NULL, 0}
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments_t *args = (struct arguments_t*)state->input;

  switch (key)
    {
    case 'r':
      {
        int errsv = errno; errno = 0;
        guard (0 == ((args->rows = strtol(arg, NULL, 0)), errno)) else
          {
            runtime_error("invalid row count: '%s'", arg);
            return 1;
          }
        errno = errsv;
        break;
      }
    case 's':
      {
        int errsv = errno; errno = 0;
        guard (0 == ((args->sparsity = strtod(arg, NULL)), errno)) else
          {
            runtime_error("invalid sparsity factor: '%s'", arg);
            return 1;
          }
        errno = errsv;
        break;
      }
    case 't':
      {
        int errsv = errno; errno = 0;
        guard (0 == ((args->threads = strtol(arg, NULL, 0)), errno)) else
          {
            runtime_error("invalid thread count: '%s'", arg);
            return 1;
          }
        errno = errsv;
        break;
      }
    case 'v':
      args->verbosity++;
      break;
    case 'q':
      args->verbosity--;
      break;
    case ARGP_KEY_ARG:
      debug_error("unexpected positional argument");
      argp_usage(state);
      break;
    case ARGP_KEY_END:
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }

  return 0;
}

static struct argp argp = {options, &parse_opt, args_doc, doc, 0, 0, 0};

struct arguments_t arguments = { DEFAULT_ROWS, DEFAULT_SPARSITY, DEFAULT_THREADS, 0 };

int
argparse (int argc, char *argv[])
{
  int res;
  debug_guard (0 == (res = argp_parse(&argp, argc, argv, 0, 0, &arguments)));
  return res;
}

