
#include "simple/main.h"

#include "util/assert.h"
#include "util/list.h"
#include "simple/threads.h"
#include "simple/data.h"
#include "simple/argparse.h"
#include "simple/topology.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void
handle_sigusr1 (int sigspec)
{
  (void)sigspec;

  printf("resetting thread counters\n");

  // TODO: reset random seed? barrier?

  size_t i;
  for (i = 0; i < threads.n; ++i)
    {
      threads.args[i].ctr = 0;
    }
}

void
handle_sigusr2 (int sigspec)
{
  (void)sigspec;

  size_t i;
  for (i = 0; i < threads.n; ++i)
    {
      printf(" + %zu: %llu\n", i, threads.args[i].ctr);
    }
}

void
handle_sigint (int sigspec)
{
  (void)sigspec;

  size_t i;
  for (i = 0; i < threads.n; ++i)
    {
      threads.args[i].cont = 0;
    }
}

int
main (int argc, char *argv[])
{
  #if !HAVE_PROGRAM_INVOCATION_NAME
  program_invocation_name = argv[0];
  #endif

  // discover NUMA topology
  int res;
  guard (0 == (res = topology_setup())) else
    {
      runtime_error("topology_setup");
      return res;
    }

  return 0;

  // parse arguments
  guard (0 == (res = argparse(argc, argv))) else
    {
      runtime_error("failed to comprehend command line arguments");
      return res;
    }

  printf("beginning setup.\n");

  srand(time(NULL));

  guard (0 == (res = data_setup(arguments.rows, (size_t)(arguments.rows * arguments.sparsity)))) else
    {
      runtime_error("data_setup");
      return res;
    }

  guard (0 == (res = threads_setup(arguments.threads))) else
    {
      runtime_error("threads_setup");
      return res;
    }

  signal(SIGUSR1, &handle_sigusr1);
  signal(SIGUSR2, &handle_sigusr2);
  signal(SIGINT, &handle_sigint);

  printf("finished setup.\n"
         "\n"
         "starting performance run.\n"
         "send:\n"
         "  SIGUSR1 - to reset counters\n"
         "  SIGUSR2 - to print counters\n"
         "  SIGINT  - to quit\n");

  guard (0 == (res = threads_join())) else
    {
      runtime_error("threads_join");
      return res;
    }

  printf("goodbye.\n");

  return 0;
}
