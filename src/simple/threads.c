#define _GNU_SOURCE

#include "simple/threads.h"
#include "simple/data.h"
#include "simple/topology.h"
#include "simple/argparse.h"

#include <errno.h>
#include <stdlib.h>
#include <sched.h>
#include <numa.h>

struct thread_list_t threads = { 0 };

static int
pin_thread (struct thread_args_t *arg)
{
  // limit execution to target cpu or node
  if (arguments.pin_strategy == PIN_STRATEGY_NODE)
    {
      debug_guard (0 == numa_run_on_node(arg->node));
    }
  else
    {
      cpu_set_t set;
      CPU_ZERO(&set);
      CPU_SET(arg->cpu, &set);
      debug_guard (0 == sched_setaffinity(0, sizeof(cpu_set_t), &set));
    }

  // limit allocations to target node
  struct bitmask *nodemask;
  guard (NULL != (nodemask = numa_allocate_nodemask())) else
    {
      runtime_error("numa_allocate_nodemask");
      return 2;
    }
  numa_bitmask_setbit(nodemask, arg->node);
  numa_set_membind(nodemask);

  return 0;
}

static void*
thread_func_linear_search (void *arg)
{
  struct thread_args_t *thread_arg = arg;

  int res;
  guard (0 == (res = pin_thread(arg))) else
    {
      runtime_error("failed to pin thread #%i", thread_arg->id);
      return NULL;
    }

  while (thread_arg->cont)
    {
      int needle = rand_r(&thread_arg->seed) % thread_arg->data_range;
      int match = data_linear_search(thread_arg->data_array, thread_arg->data_rows, needle);
      (void)match;
      thread_arg->ctr++;
    }

  return NULL;
}

static void*
thread_func_tree_search (void *arg)
{
  struct thread_args_t *thread_arg = arg;

  int res;
  guard (0 == (res = pin_thread(arg))) else
    {
      runtime_error("failed to pin thread #%i", thread_arg->id);
      return NULL;
    }

  while (thread_arg->cont)
    {
      int needle = rand_r(&thread_arg->seed) % thread_arg->data_range;
      int match = data_tree_search(thread_arg->tree, needle);
      (void)match;
      thread_arg->ctr++;
    }

  return NULL;
}

static void
thread_setup_shared (struct thread_args_t *args)
{
  args->ctr = 0;
  args->seed = threads.seed;
  args->data_range = data_range;
  args->cont = 1;
}

int
threads_setup (void)
{
  printf("starting threads ...\n");

  size_t n = 0;
  size_t i;
  for (i = 0; i < topology.nodes.n; ++i)
    {
      if (arguments.pin_strategy == PIN_STRATEGY_CPU)
        {
          n += topology.nodes.nodes[i].cpus.n;
        }
      else
        {
          n += topology.nodes.nodes[i].cpus.n + 1;
        }
    }

  threads.n = n;

  guard (NULL != (threads.threads = malloc(sizeof(*threads.threads) * threads.n))) else { return 2; }
  guard (NULL != (threads.args = malloc(sizeof(*threads.args) * threads.n))) else { return 2; }

  threads.seed = time(NULL);

  void*(*thread_func)(void*) = NULL;
  const char *_thread_func = NULL;
  if (arguments.tree_search)
    {
      thread_func = &thread_func_tree_search;
      _thread_func = "tree_search";
    }
  else
    {
      thread_func = &thread_func_linear_search;
      _thread_func = "linear_search";
    }

  size_t j;
  for (n = 0, i = 0; i < topology.nodes.n; ++i)
    {
      if (arguments.pin_strategy == PIN_STRATEGY_CPU)
        {
          for (j = 0; j < topology.nodes.nodes[i].cpus.n; ++j)
            {
              printf("  provisioning thread #%zu running %s on CPU #%i\n", n, _thread_func, topology.nodes.nodes[i].cpus.cpus[j]);
              threads.args[n].id = n;
              threads.args[n].node = topology.nodes.nodes[i].num;
              threads.args[n].cpu = topology.nodes.nodes[i].cpus.cpus[j];

              if (arguments.tree_search)
                {
                  if (arguments.replicate)
                    {
                      threads.args[n].tree = topology.nodes.nodes[i].replica;
                    }
                  else
                    {
                      threads.args[i].tree = data_tree;
                    }
                }
              else
                {
                  threads.args[i].data_array = data_array;
                  threads.args[i].data_rows = data_rows;
                }

              thread_setup_shared(threads.args + n++);
            }
        }
      else // PIN_STRATEGY_NODES
        {
          for (j = 0; j < topology.nodes.nodes[i].cpus.n + 1; ++j)
            {
              printf("  provisioning thread #%zu running %s on NODE #%i\n", n, _thread_func, topology.nodes.nodes[i].num);
              threads.args[n].id = n;
              threads.args[n].node = topology.nodes.nodes[i].num;
              threads.args[n].cpu = -1;

              if (arguments.tree_search)
                {
                  if (arguments.replicate)
                    {
                      threads.args[n].tree = topology.nodes.nodes[i].replica;
                    }
                  else
                    {
                      threads.args[i].tree = data_tree;
                    }
                }
              else
                {
                  threads.args[i].data_array = data_array;
                  threads.args[i].data_rows = data_rows;
                }

              thread_setup_shared(threads.args + n++);
            }
        }
    }

  int res;
  for (i = 0; i < threads.n; ++i)
    {
      guard (0 == (res = pthread_create(threads.threads + i, NULL, thread_func, threads.args + i))) else { return res; }
    }

  return 0;
}

int threads_join (void)
{
  size_t i;
  for (i = 0; i < threads.n; ++i)
    {
      int res;
      guard (0 == (res = pthread_join(threads.threads[i], NULL))) else { return res; }
    }

  free(threads.threads);
  threads.threads = NULL;
  free(threads.args);
  threads.args = NULL;

  threads.n = 0;

  return 0;
}
