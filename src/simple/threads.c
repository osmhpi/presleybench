
#include "simple/threads.h"
#include "simple/data.h"

#include <errno.h>
#include <stdlib.h>

struct thread_list_t threads = { 0 };

static void*
thread_func (void *arg)
{
  struct thread_args_t *thread_arg = arg;

  while (thread_arg->cont)
    {
      int needle = rand_r(&thread_arg->seed) % data_range;
      int match = data_linear_search(needle);
      (void)match;
    }

  return NULL;
}

int
threads_setup (size_t n)
{
  guard (n > 0) else
    {
      errno = EINVAL;
      return 1;
    }

  threads.n = n;

  guard (NULL != (threads.threads = malloc(sizeof(*threads.threads) * threads.n))) else { return 2; }
  guard (NULL != (threads.args = malloc(sizeof(*threads.args) * threads.n))) else { return 2; }

  threads.seed = time(NULL);

  size_t i;
  for (i = 0; i < threads.n; ++i)
    {
      threads.args[i].seed = threads.seed;
      threads.args[i].ctr = 0;
      threads.args[i].cont = 1;

      int res;
      guard (0 == (res = pthread_create(threads.threads + i, NULL, &thread_func, threads.args + i))) else { return res; }
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
