#define _GNU_SOURCE

#include "simple/main.h"

#include "util/assert.h"
#include "simple/threads.h"
#include "simple/data.h"
#include "simple/argparse.h"
#include "simple/topology.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sched.h>
#include <numa.h>

void
handle_sigusr1 (int sigspec)
{
  (void)sigspec;

  printf("resetting thread counters\n");

  // TODO: reset random seed? barrier?

  size_t i;
  for (i = 0; i < threads.n; ++i)
    {
      threads.args[i].round++;
      threads.args[i].ctr = 0;
    }
}

void
handle_sigusr2 (int sigspec)
{
  (void)sigspec;

  printf("threadid,nodeid,cpuid,round,counter\n");

  size_t i;
  for (i = 0; i < threads.n; ++i)
    {
      printf(" %i,%i,%i,%i,%llu\n",
             threads.args[i].id,
             threads.args[i].node,
             threads.args[i].cpu,
             threads.args[i].round,
             threads.args[i].ctr);
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

  // test for numa
  guard (0 == numa_available()) else
    {
      runtime_error("NUMA capabilities not available.");
      return 1;
    }

  // parse arguments
  int res;
  guard (0 == (res = argparse(argc, argv))) else
    {
      runtime_error("failed to comprehend command line arguments");
      return res;
    }

  if (arguments.primary_node < 0)
    {
      arguments.primary_node = numa_node_of_cpu(sched_getcpu());
    }

  // limit allocations to primary node
  numa_set_strict(1);
  fprintf(stderr, "attempting to bind master task allocations to node #%i\n", arguments.primary_node);
  guard (0 == (res = numa_membind_to_node(arguments.primary_node))) else
    {
      runtime_error("failed to bind memory allocations to node #%i", arguments.primary_node);
      return res;
    }

  // discover NUMA topology
  guard (0 == (res = topology_setup())) else
    {
      runtime_error("topology_setup");
      return res;
    }

  if (arguments.primary_node == -1)
    {
      arguments.primary_node = topology.nodes.nodes[0].num;
    }

  guard (NULL != topology_node_get(arguments.primary_node)) else
    {
      runtime_error("unknown primary node: '%s'", arguments._primary_node);
      return 1;
    }

  srand(time(NULL));

  guard (0 == (res = data_setup(arguments.rows, (size_t)(arguments.rows * arguments.sparsity)))) else
    {
      runtime_error("data_setup");
      return res;
    }

  guard (0 == (res = threads_setup())) else
    {
      runtime_error("threads_setup");
      return res;
    }

  signal(SIGUSR1, &handle_sigusr1);
  signal(SIGUSR2, &handle_sigusr2);
  signal(SIGINT, &handle_sigint);

  fprintf(stderr,
          "finished setup.\n"
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

  fprintf(stderr, "goodbye.\n");

  return 0;
}
