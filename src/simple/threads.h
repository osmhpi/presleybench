#ifndef THREADS_H
#define THREADS_H

#include "util/assert.h"

#include <pthread.h>

struct thread_args_t
{
  unsigned long long ctr;
  unsigned int seed;

  int cont;
};

struct thread_list_t
{
  unsigned int seed;

  size_t n;
  pthread_t *threads;
  struct thread_args_t *args;
};

extern struct thread_list_t threads;

int threads_setup(size_t) att_warn_unused_result;
int threads_join(void) att_warn_unused_result;

#endif
