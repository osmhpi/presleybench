
#include "simple/main.hpp"

#include "util/assert.hpp"
#include "simple/threads.hpp"
#include "simple/data.hpp"
#include "simple/argparse.hpp"

#include <sys/sysinfo.h>

#include <iostream>
#include <csignal>

std::vector<struct thread_args_t*> thread_args;

void
handle_sigusr1 (_presley_unused int sigspec)
{
  std::cout << "threadid,nodeid,cpuid,round,counter" << std::endl;

  size_t i;
  for (i = 0; i < thread_args.size(); ++i)
    {
      std::cout <<
         thread_args[i]->id << "," <<
         thread_args[i]->node << "," <<
         thread_args[i]->cpu << "," <<
         thread_args[i]->round << "," <<
         *(thread_args[i]->ctr) - thread_args[i]->prev_ctr << std::endl;
    }
}

void
handle_sigusr2 (_presley_unused int sigspec)
{
  std::cout << "resetting thread counters" << std::endl;

  size_t i;
  for (i = 0; i < thread_args.size(); ++i)
    {
      thread_args[i]->round++;
      thread_args[i]->prev_ctr = *(thread_args[i]->ctr);
    }
}

void
handle_sigint (_presley_unused int sigspec)
{
  std::cerr << "sending all threads the stop signal" << std::endl;

  size_t i;
  for (i = 0; i < thread_args.size(); ++i)
    {
      // gracefully stop all worker threads
      thread_args[i]->cont = 0;
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
      presley_runtime_error("failed to comprehend command line arguments");
      return res;
    }

  // discover NUMA topology
  std::cerr << "discovered topology information:" << std::endl;
  for (numa::Node node : numa::NodeList::logicalNodesWithCPUs())
    {
      std::cerr << "  node #" << node.logicalId() << std::endl;
      std::cerr << "  cpus:" << std::endl;

      std::cerr << "  ";
      for (numa::CpuId cpuid : node.cpuids())
        std::cerr << "  " << cpuid;
      std::cerr << std::endl;
    }

  // seed PRNG
  srand(time(NULL));

  // generate test data
  guard (0 == (res = data_setup(arguments.rows, (size_t)(arguments.rows * arguments.sparsity)))) else
    {
      presley_runtime_error("data_setup");
      return res;
    }

  // start worker threads
  std::cerr << "starting " << get_nprocs() << " threads on "
      << get_nprocs() << " cpus across "
      << numa::NodeList::logicalNodesWithCPUsCount() << " nodes..." << std::endl;

  std::atomic_size_t threadCount(0);
  int seed = time(NULL);

  for (int i = 0; i < get_nprocs(); ++i)
    {
      thread_args.push_back(NULL);
    }

  const char *_thread_func = arguments.index_search ? "index_search" : "linear_search";
  auto threads = numa::forEachThread(numa::NodeList::logicalNodesWithCPUs(), [&]()
    {
      numa::Node currentNode = numa::Node::curr();

      size_t thread_id = threadCount.fetch_add(1);
      struct thread_args_t *args = new(struct thread_args_t);

      args->id = thread_id;
      args->node = currentNode.logicalId();
      args->cpu = currentNode.currCpuid();

      std::cerr << "  provisioning thread #" << thread_id
          << " running " << _thread_func
          << " on Node #" << args->node
          << ", CPU #" << args->cpu << std::endl;

      args->data_array = data_array;
      args->data_rows = data_rows;

      args->round = 0;
      args->prev_ctr = 0;
      args->ctr = new(unsigned long long);

      *args->ctr = 0;
      args->seed = seed;
      args->data_range = data_range;
      args->cont = 1;

      if (!arguments.index_search)
        args->index = NULL;
      else if (arguments.replicate)
        args->index = data_index[args->node];
      else
        args->index = data_index[primaryNode.logicalId()];

      thread_args[thread_id] = args;

      if (arguments.index_search)
        thread_func_index_search(args);
      else
        thread_func_linear_search(args);
    }, 0);

  // setup signal handlers
  std::signal(SIGUSR1, &handle_sigusr1);
  std::signal(SIGUSR2, &handle_sigusr2);
  std::signal(SIGINT, &handle_sigint);

  // all done. print usage notice
  std::cerr <<
      "finished setup." << std::endl <<
      std::endl <<
      "starting performance run." << std::endl <<
      "send:" << std::endl <<
      "  SIGUSR2 - to reset counters" << std::endl <<
      "  SIGUSR1 - to print counters" << std::endl <<
      "  SIGINT  - to quit" << std::endl;

  // wait for workers to terminate
  numa::wait(threads);

  std::cerr << "goodbye." << std::endl;

  return 0;
}
