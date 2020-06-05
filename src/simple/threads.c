#define _GNU_SOURCE

#include "simple/threads.h"
#include "simple/data.h"
#include "simple/topology.h"
#include "simple/argparse.h"

#include <errno.h>
#include <stdlib.h>

struct thread_list_t threads = { 0 };

static int
pin_thread (struct thread_args_t *arg)
{
  // limit execution to target cpu or node
  int res;
  if (arguments.pin_strategy == PIN_STRATEGY_NODE)
    {
      guard (0 == (res = topology_pin_to_node(arg->node))) else { return res; }
    }
  else
    {
      guard (0 == (res = topology_pin_to_cpu(arg->node))) else { return res; }
    }

  guard (0 == (res = topology_membind_to_node(arg->node))) else { return res; }

  return 0;
}

static void*
thread_func_linear_search (void *arg)
{
  //void *frame = __builtin_frame_address(0);
  //int node;
  //debug_guard (0 == numa_move_pages(0, 1, &frame, NULL, &node, 0));

  struct thread_args_t *thread_arg = arg;

  //fprintf(stderr, "thread #%i: pre-pin node of stack data: #%i\n", thread_arg->id, node);

  int res;
  guard (0 == (res = pin_thread(arg))) else
    {
      runtime_error("failed to pin thread #%i", thread_arg->id);
      return NULL;
    }

  //frame = __builtin_frame_address(0);
  //debug_guard (0 == numa_move_pages(0, 1, &frame, NULL, &node, 0));

  //fprintf(stderr, "thread #%i: post-pin node of stack data: #%i\n", thread_arg->id, node);

  while (thread_arg->cont)
    {
      int needle = rand_r(&thread_arg->seed) % thread_arg->data_range;
      int match = data_linear_search(thread_arg->data_array, thread_arg->data_rows, needle);

      if (arguments.verify)
        {
          guard (match == -1 || thread_arg->data_array[match] == needle) else
            {
              runtime_error("thread #%i:[%llu] search produced incorrect result. %i[%i] vs %i, Aborting.", thread_arg->id, thread_arg->ctr, thread_arg->data_array[match], match, needle);
              return NULL;
            }
        }

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

      if (arguments.verify)
        {
          guard (match == -1 || thread_arg->data_array[match] == needle) else
            {
              int real_match = data_linear_search(thread_arg->data_array, thread_arg->data_rows, needle);
              runtime_error("thread #%i:[%llu] search produced incorrect result. %i[%i] vs %i[%i], Aborting.", thread_arg->id, thread_arg->ctr, thread_arg->data_array[match], match, needle, real_match);
              return NULL;
            }
        }

      thread_arg->ctr++;
    }

  return NULL;
}

static void
thread_setup_shared (struct thread_args_t *args)
{
  args->round = 0;
  args->ctr = 0;
  args->seed = threads.seed;
  args->data_range = data_range;
  args->cont = 1;
}

int
threads_setup (void)
{
  fprintf(stderr, "starting threads ...\n");

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
              fprintf(stderr, "  provisioning thread #%zu running %s on CPU #%i\n", n, _thread_func, topology.nodes.nodes[i].cpus.cpus[j]);
              threads.args[n].id = n;
              threads.args[n].node = topology.nodes.nodes[i].num;
              threads.args[n].cpu = topology.nodes.nodes[i].cpus.cpus[j];

              threads.args[n].data_array = data_array;
              threads.args[n].data_rows = data_rows;
              if (arguments.tree_search)
                {
                  if (arguments.replicate)
                    {
                      threads.args[n].tree = topology.nodes.nodes[i].replica;
                    }
                  else
                    {
                      threads.args[n].tree = data_tree;
                    }
                }

              thread_setup_shared(threads.args + n++);
            }
        }
      else // PIN_STRATEGY_NODES
        {
          for (j = 0; j < topology.nodes.nodes[i].cpus.n + 1; ++j)
            {
              fprintf(stderr, "  provisioning thread #%zu running %s on NODE #%i\n", n, _thread_func, topology.nodes.nodes[i].num);
              threads.args[n].id = n;
              threads.args[n].node = topology.nodes.nodes[i].num;
              threads.args[n].cpu = -1;

              threads.args[n].data_array = data_array;
              threads.args[n].data_rows = data_rows;
              if (arguments.tree_search)
                {
                  if (arguments.replicate)
                    {
                      threads.args[n].tree = topology.nodes.nodes[i].replica;
                    }
                  else
                    {
                      threads.args[n].tree = data_tree;
                    }
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
