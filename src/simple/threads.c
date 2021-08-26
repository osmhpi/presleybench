#define _GNU_SOURCE

#include "simple/threads.h"
#include "simple/data.h"
#include "simple/topology.h"
#include "simple/argparse.h"

#if HAVE_NUMA
#  include <numa.h>
#else
#  define numa_alloc_onnode(S, N) malloc(S)
#  define numa_free(P, S) free(P)
#endif

#if defined(HAVE_LTTNG) && defined(TRACEPOINTS_ENABLED)
#  define TRACEPOINT_DEFINE
#  include "simple/presleybench_simple_threads.h"
#endif

#include <errno.h>
#include <stdlib.h>

#define CHECK_INTERVAL 0x10000

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
      guard (0 == (res = topology_pin_to_cpu(arg->cpu))) else { return res; }
    }

  guard (0 == (res = topology_membind_to_node(arg->node))) else { return res; }

  return 0;
}

void*
thread_func_linear_search (void *arg)
{
  struct thread_args_t *thread_arg = arg;

  int res;
  guard (0 == (res = pin_thread(arg))) else
    {
      presley_runtime_error("failed to pin thread #%i", thread_arg->id);
      return NULL;
    }

  int range = thread_arg->data_range;
  int needle = rand_r(&thread_arg->seed) % range;
  int step = rand_r(&thread_arg->seed) % range;
  int rows = thread_arg->data_rows;
  int *array = thread_arg->data_array;
  int verify = arguments.verify;
  unsigned long long *ctr = thread_arg->ctr;

  while (1)
    {
      needle = (needle + step) % range;
      int match = data_linear_search(array, rows, needle);

#if DEBUG
      fprintf(stderr, "%i: %i (%i)\n", needle, match, match >= 0 ? data_array[match] : -1);
#endif

      if (verify)
        {
          guard (match == -1 || array[match] == needle) else
            {
              presley_runtime_error("thread #%i:[%llu] search produced incorrect result. %i[%i] vs %i, Aborting.",
                            thread_arg->id, *thread_arg->ctr, thread_arg->data_array[match], match, needle);
              return NULL;
            }
        }

      (*ctr)++;

      if ((*ctr) % CHECK_INTERVAL == 0 && !thread_arg->cont)
        {
#if DEBUG
          fprintf(stderr, "thread #%i: break condition reached\n", thread_arg->id);
#endif
          break;
        }
    }

  return NULL;
}

void*
thread_func_index_search (void *arg)
{
  struct thread_args_t *thread_arg = arg;

  int res;
  guard (0 == (res = pin_thread(arg))) else
    {
      presley_runtime_error("failed to pin thread #%i", thread_arg->id);
      return NULL;
    }

  fprintf(stderr, "thread #%i with data %p\n", thread_arg->id, thread_arg);

  int range = thread_arg->data_range;
  int needle = rand_r(&thread_arg->seed) % range;
  int step = rand_r(&thread_arg->seed) % range;
  int verify = arguments.verify;
  unsigned long long *ctr = thread_arg->ctr;
  struct index_t *index = thread_arg->index;

  while (1)
    {
      needle = (needle + step) % range;
      int match = data_index_search(index, needle);

#if DEBUG
      fprintf(stderr, "%i: %i (%i)\n", needle, match, match >= 0 ? data_array[match] : -1);
#endif

      if (verify)
        {
          guard (match == -1 || thread_arg->data_array[match] == needle) else
            {
              int real_match = data_linear_search(thread_arg->data_array, thread_arg->data_rows, needle);
              presley_runtime_error("thread #%i:[%llu] search produced incorrect result. %i[%i] vs %i[%i], Aborting.",
                            thread_arg->id, *thread_arg->ctr, thread_arg->data_array[match - 1], match - 1, needle, real_match);
              return NULL;
            }
        }

      (*ctr)++;

      if ((*ctr) % CHECK_INTERVAL == 0 && !thread_arg->cont)
        break;
    }

  return NULL;
}

int
threads_setup (void)
{
  threads.n = topology_cpu_count();

  fprintf(stderr, "starting %zu threads on %zu cpus across %zu nodes...\n", threads.n, threads.n, topology.nodes.n);

  guard (NULL != (threads.threads = malloc(sizeof(*threads.threads) * threads.n))) else { return 2; }
  guard (NULL != (threads.args = malloc(sizeof(*threads.args) * threads.n))) else { return 2; }

  threads.seed = time(NULL);

  void*(*thread_func)(void*) = arguments.index_search ? &thread_func_index_search : thread_func_linear_search;
  const char *_thread_func = arguments.index_search ? "index_search" : "linear_search";

  size_t i, j, n;
  for (i = 0, n = 0; i < topology.nodes.n; ++i)
    for (j = 0; j < topology.nodes.nodes[i].cpus.n; ++j, ++n)
      {
        if (arguments.pin_strategy == PIN_STRATEGY_NODE)
          fprintf(stderr, "  provisioning thread #%zu running %s on NODE #%i\n", n, _thread_func, topology.nodes.nodes[i].num);
        else
          fprintf(stderr, "  provisioning thread #%zu running %s on CPU #%i\n", n, _thread_func, topology.nodes.nodes[i].cpus.cpus[j]);

        threads.args[n].id = n;
        threads.args[n].node = topology.nodes.nodes[i].num;
        threads.args[n].cpu = topology.nodes.nodes[i].cpus.cpus[j];

        threads.args[n].data_array = data_array;
        threads.args[n].data_rows = data_rows;

        threads.args[n].round = 0;
        threads.args[n].prev_ctr = 0;
        guard (NULL != (threads.args[n].ctr = numa_alloc_onnode(sizeof(*threads.args[n].ctr), topology.nodes.nodes[i].num)));
        *threads.args[n].ctr = 0;
        threads.args[n].seed = threads.seed;
        threads.args[n].data_range = data_range;
        threads.args[n].cont = 1;

        if (!arguments.index_search)
          threads.args[n].index = NULL;
        else if (arguments.replicate)
          threads.args[n].index = data_index[primary_node];
        else
          threads.args[n].index = data_index[topology.nodes.nodes[i].num];
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
