#ifndef MAIN_H
#define MAIN_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <argp.h>

const char *argp_program_version = PACKAGE_STRING;
const char *argp_program_bug_address = PACKAGE_BUGREPORT;

const char doc[] = PACKAGE_NAME " - a simple microbenchmark tool";
const char args_doc[] = "[--] <schema file>";

static struct argp_option options[] =
{
  {"verbose", 'v', 0, 0, "be more verbose", 0},
  {"quiet", 'q', 0, 0, "be less verbose", 0},
  {0, 0, 0, 0, 0, 0}
};

struct arguments
{
  const char *schemafile;
  int verbosity;
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *args = (struct arguments*)state->input;

  switch (key)
    {
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
