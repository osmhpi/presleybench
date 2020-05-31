
#include "argparse.h"

#include "util/assert.h"
#include "schema/tpcc.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

const char *argp_program_version = PACKAGE_STRING;
const char *argp_program_bug_address = PACKAGE_BUGREPORT;

const char doc[] = PACKAGE_NAME " - a simple microbenchmark tool";
const char args_doc[] = "[--] <schema file>";

static struct argp_option options[] =
{
  {NULL, 't', NULL, 0, "write the tpc-c schema to stdout and exit", 0},
  {"scale", 's', "factor", 0, "scale factor for tpc-c, must be one of 1, 10, 100, 300, 1000, 3000, 10000, 30000, 100000", 0},
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
    case 't':
      puts(default_tpcc);
      exit(0);
    case 's':
      {
        int errsv = errno; errno = 0;
        guard (0 == ((args->scale = strtol(arg, NULL, 0)), errno)) else
          {
            runtime_error("invalid scale factor: '%s'", arg);
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
      if (state->arg_num > 1)
        {
          debug_error("unexpected positional argument");
          argp_usage(state);
        }
      args->schemafile = arg;
      break;
    case ARGP_KEY_END:
      if (state->arg_num != 1)
        {
          debug_error("unexpected end of arguments");
          argp_usage(state);
        }
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }

  return 0;
}

static struct argp argp = {options, &parse_opt, args_doc, doc, 0, 0, 0};

struct arguments_t arguments = { NULL, 0, 1 };

int
argparse (int argc, char *argv[])
{
  int res;
  debug_guard (0 == (res = argp_parse(&argp, argc, argv, 0, 0, &arguments)));
  return res;
}
