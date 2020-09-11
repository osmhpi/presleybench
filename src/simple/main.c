
#include "simple/main.h"

#include "util/assert.h"
#include "simple/threads.h"
#include "simple/data.h"
#include "simple/argparse.h"
#include "simple/topology.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

// TODO: fix race condition in reset

void
handle_sigusr1 (int sigspec)
{
  (void)sigspec;

  printf("threadid,nodeid,cpuid,round,counter\n");

  size_t i;
  for (i = 0; i < threads.n; ++i)
    {
      printf("%i,%i,%i,%i,%llu\n",
             threads.args[i].id,
             threads.args[i].node,
             threads.args[i].cpu,
             threads.args[i].round,
             threads.args[i].ctr - threads.args[i].prev_ctr);
    }
}

void
handle_sigusr2 (int sigspec)
{
  (void)sigspec;

  printf("resetting thread counters\n");

  // TODO: reset random seed? barrier?

  size_t i;
  for (i = 0; i < threads.n; ++i)
    {
      threads.args[i].round++;
      threads.args[i].prev_ctr = threads.args[i].ctr;
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
  #ifndef HAVE_PROGRAM_INVOCATION_NAME
  program_invocation_name = argv[0];
  #endif

  // parse arguments
  int res;
  guard (0 == (res = argparse(argc, argv))) else
    {
      runtime_error("failed to comprehend command line arguments");
      return res;
    }

  // discover NUMA topology
  guard (0 == (res = topology_setup())) else
    {
      runtime_error("topology_setup");
      return res;
    }

  // seed PRNG
  srand(time(NULL));

  // generate test data
  guard (0 == (res = data_setup(arguments.rows, (size_t)(arguments.rows * arguments.sparsity)))) else
    {
      runtime_error("data_setup");
      return res;
    }

  // start worker threads
  guard (0 == (res = threads_setup())) else
    {
      runtime_error("threads_setup");
      return res;
    }

  // setup signal handlers
  signal(SIGUSR1, &handle_sigusr1);
  signal(SIGUSR2, &handle_sigusr2);
  signal(SIGINT, &handle_sigint);

  // all done. print usage notice
  fprintf(stderr,
          "finished setup.\n"
          "\n"
          "starting performance run.\n"
          "send:\n"
          "  SIGUSR2 - to reset counters\n"
          "  SIGUSR1 - to print counters\n"
          "  SIGINT  - to quit\n");

  // wait for workers to terminate
  guard (0 == (res = threads_join())) else
    {
      runtime_error("threads_join");
      return res;
    }

  fprintf(stderr, "goodbye.\n");

  return 0;
}
