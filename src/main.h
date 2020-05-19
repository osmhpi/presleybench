#ifndef MAIN_H
#define MAIN_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "schema/tpcc.h"

#include <argp.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

extern const char *program_invocation_name;

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

struct arguments
{
  const char *schemafile;
  int verbosity;
  unsigned int scale;
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *args = (struct arguments*)state->input;

  switch (key)
    {
    case 't':
      puts(default_tpcc);
      exit(0);
    case 's':
      {
        int errsv = errno;
        errno = 0;
        unsigned int scale = strtol(arg, NULL, 0);
        if (errno)
          {
            fprintf(stderr, "%s: invalid scale factor: '%s': %s\n", program_invocation_name, arg, strerror(errno));
            exit(1);
          }
        errno = errsv;
        args->scale = scale;
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
        argp_usage(state);
      args->schemafile = arg;
      break;
    case ARGP_KEY_END:
      if (state->arg_num != 1)
        argp_usage(state);
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }

  return 0;
}

static struct argp argp = {options, &parse_opt, args_doc, doc, 0, 0, 0};

#endif
