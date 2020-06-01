#ifndef THREADS_H
#define THREADS_H

#include "util/assert.h"
#include "simple/bplustree.h"

#include <pthread.h>

struct thread_args_t
{
  int id;

  unsigned long long ctr;
  unsigned int seed;

  size_t data_range;

  union {
    struct {
      int *data_array;
      size_t data_rows;
    };
    struct bplus_tree *tree;
  };

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

#endif
