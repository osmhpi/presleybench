#define _GNU_SOURCE

#include "simple/threads.hpp"
#include "simple/data.hpp"
#include "simple/argparse.hpp"

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

#include <cerrno>
#include <cstdlib>
#include <iostream>

#define CHECK_INTERVAL 0x10000

void*
thread_func_linear_search (void *arg)
{
  struct thread_args_t *thread_arg = (struct thread_args_t*) arg;

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
      std::cerr << needle << ": " << match << " (" << match >= 0 ? data_array[match] : -1 << ")" << std::endl;
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
          std::cerr << "thread #" << thread_arg->id << ": break condition reached" << std::endl;
#endif
          break;
        }
    }

  return NULL;
}

void*
thread_func_index_search (void *arg)
{
  struct thread_args_t *thread_arg = (struct thread_args_t*) arg;

  int res;

  std::cerr << "thread #" << thread_arg->id << " with data " << thread_arg << std::endl;

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
      std::cerr << needle << ": " << match << " (" << match >= 0 ? data_array[match] : -1 << ")" << std::endl;
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
