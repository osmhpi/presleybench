
#include "argparse.h"

#include "util/assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define DEFAULT_ROWS 100000000
#define DEFAULT_SPARSITY 2.0

const char *argp_program_version = PACKAGE_STRING;
const char *argp_program_bug_address = PACKAGE_BUGREPORT;

const char doc[] = PACKAGE_NAME " - a simple microbenchmark tool";
const char args_doc[] = "[--]";

static struct argp_option options[] =
{
  {"primary", 'p', "<node>", 0, "the id of the numa node for the primary data placement", 0},
  {"replicate", 'e', NULL, 0, "replicate the index data across nodes (implies -i)", 0},
  {"verify", 'V', NULL, 0, "perform additional checks during the number crunching. useful for testing index implementations.", 0 },
  {"index", 'i', NULL, 0, "perform index search instead of linear scan", 0},
  {"index-type", 'I', "<bplus|groupkey|...>", 0, "the type of index to use. default: bplus (implies -i)", 0},
  {"rows", 'r', "<number>", 0, "the amount of rows to populate", 0},
  {"sparsity", 's', "<fraction>", 0, "the sparsity of populated values. must be >= 1", 0},
  {"pin-strategy", 'P', "<cpu|node>", 0, "what level of topology to pin threads to. default: cpu", 0},
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
    case 'p':
      {
        int errsv = errno; errno = 0;
        guard (0 == ((args->primary_node = strtol(arg, NULL, 0)), errno)) else
          {
            presley_runtime_error("invalid primary node number: '%s'", arg);
            return 1;
          }
        arguments._primary_node = arg;
        errno = errsv;
        break;
      }
    case 'e':
      args->replicate = 1;
      args->index_search = 1;
      break;
    case 'V':
      args->verify = 1;
      break;
    case 'i':
      args->index_search = 1;
      break;
    case 'I':
      args->index_search = 1;
      if (!strcmp("bplus", arg))
        {
          args->index_type = INDEX_TYPE_BPLUS;
        }
      else if (!strcmp("groupkey", arg))
        {
          args->index_type = INDEX_TYPE_GROUPKEY;
        }
      else
        {
          presley_runtime_error("invalid index type: '%s'", arg);
          return 1;
        }
      args->_index_type = arg;
      break;
    case 'r':
      {
        int errsv = errno; errno = 0;
        guard (0 == ((args->rows = strtol(arg, NULL, 0)), errno)) else
          {
            presley_runtime_error("invalid row count: '%s'", arg);
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
            presley_runtime_error("invalid sparsity factor: '%s'", arg);
            return 1;
          }
        errno = errsv;
        break;
      }
    case 'P':
      if (!strcmp("cpu", arg))
        {
          args->pin_strategy = PIN_STRATEGY_CPU;
        }
      else if (!strcmp("node", arg))
        {
          args->pin_strategy = PIN_STRATEGY_NODE;
        }
      else
        {
          presley_runtime_error("invalid pin strategy: '%s'", arg);
          return 1;
        }
      args->_pin_strategy = arg;
      break;
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

struct arguments_t arguments = { DEFAULT_ROWS, DEFAULT_SPARSITY, -1, NULL, 0, 0, INDEX_TYPE_BPLUS, NULL, PIN_STRATEGY_CPU, NULL, 0, 0 };

int
argparse (int argc, char *argv[])
{
  int res;
  debug_guard (0 == (res = argp_parse(&argp, argc, argv, 0, 0, &arguments)));
  return res;
}

