#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "util/assert.h"

#include <stddef.h>

struct node_t
{
  int num;
  struct {
    size_t n;
    int *cpus;
  } cpus;
};

struct topology_t
{
  struct {
    size_t n;
    struct node_t *nodes;
  } nodes;
};

extern struct topology_t topology;

int topology_setup(void) att_warn_unused_result;

struct node_t *topology_node_get(int) att_warn_unused_result;

#endif
