#ifndef THREADS_H
#define THREADS_H

#include "util/assert.hpp"
#include "simple/index/index.hpp"

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

void* thread_func_index_search(void*);
void* thread_func_linear_search(void*);

#endif
