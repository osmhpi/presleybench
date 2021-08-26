#ifndef THREADS_H
#define THREADS_H

#include "util/assert.h"
#include "simple/index/index.h"

#include <pthread.h>

struct thread_args_t
{
  int id;
  int round;

  unsigned long long prev_ctr;
  unsigned long long *ctr;
  unsigned int seed;

  size_t data_range;

  struct {
    int *data_array;
    size_t data_rows;
  };
  struct index_t *index;

  int cont;

  int node;
  int cpu;
};

struct thread_list_t
{
  unsigned int seed;

  size_t n;
  pthread_t *threads;
  struct thread_args_t *args;
};

extern struct thread_list_t threads;

int threads_setup(void) att_warn_unused_result;
int threads_join(void) att_warn_unused_result;

void* thread_func_index_search(void*);
void* thread_func_linear_search(void*);

#endif
