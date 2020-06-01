
#include "simple/topology.h"
#include "simple/argparse.h"

#include <numa.h>

#include <stdio.h>

struct topology_t topology = { 0 };

struct node_t* att_warn_unused_result
topology_node_get (int nodenum)
{
  size_t i;
  for (i = 0; i < topology.nodes.n; ++i)
    {
      if (topology.nodes.nodes[i].num == nodenum)
        return topology.nodes.nodes + i;
    }
  return NULL;
}

static int att_warn_unused_result
node_add_cpu (struct node_t* node, int cpunum)
{
  int *cpus;
  guard (NULL != (cpus = realloc(node->cpus.cpus, sizeof(*cpus) * (node->cpus.n + 1)))) else { return 2; }

  cpus[node->cpus.n] = cpunum;
  node->cpus.cpus = cpus;
  node->cpus.n++;

  return 0;
}

int
topology_setup (void)
{
  //printf("numa_max_possible_node: %i\n", numa_max_possible_node());
  //printf("numa_num_possible_nodes: %i\n", numa_num_possible_nodes());
  //printf("numa_max_node: %i\n", numa_max_node());

  topology.nodes.n = numa_num_configured_nodes();
  printf("numa_num_configured_nodes: %zu\n", topology.nodes.n);

  guard (NULL != (topology.nodes.nodes = malloc(sizeof(*topology.nodes.nodes) * topology.nodes.n))) else
    {
      runtime_error("malloc");
      return 2;
    }

  memset(topology.nodes.nodes, 0, sizeof(*topology.nodes.nodes) * topology.nodes.n);

  struct bitmask *mask = numa_all_nodes_ptr;
  //printf("mask size: %lu\n", mask->size);

  unsigned long i, v, x, j;
  printf("node mask: ");
  for (i = 0, v = 0, x = 0, j = 0; i < mask->size; ++i)
    {
      if (!(i % (sizeof(unsigned long) * 8)))
        {
          v = mask->maskp[x++];
          if (i > 0)
            printf("\n           ");
        }
      if (v & 1)
        topology.nodes.nodes[j++].num = i;
      printf("%lu", v & 1);
      v >>= 1;
    }
  printf("\n");

  int num_configured_cpus = numa_num_configured_cpus();
  printf("numa_num_configured_cpus: %i\n", num_configured_cpus);

  mask = numa_all_cpus_ptr;
  //printf("mask size: %lu\n", mask->size);

  printf("cpu mask : ");
  for (i = 0, v = 0, x = 0, j = 0; i < mask->size; ++i)
    {
      if (!(i % (sizeof(unsigned long) * 8)))
        {
          v = mask->maskp[x++];
          if (i > 0)
            printf("\n           ");
        }
      if (v & 1)
        {
          int nodenum = numa_node_of_cpu(i);
          struct node_t *node = topology_node_get(nodenum);
          int res;
          guard (0 == (res = node_add_cpu(node, i))) else { return res; }
        }
      printf("%lu", v & 1);
      v >>= 1;
    }
  printf("\n");

  printf("discovered topology information:\n");

  for (i = 0; i < topology.nodes.n; ++i)
    {
      printf("  node #%i\n", topology.nodes.nodes[i].num);
      printf("  cpus:");
      for (j = 0; j < topology.nodes.nodes[i].cpus.n; ++j)
        {
          printf(" %i", topology.nodes.nodes[i].cpus.cpus[j]);
        }
      printf("\n");
    }

  //return 0;

  return 0;
}

int
numa_membind_to_node (int node)
{
  struct bitmask *nodemask;
  guard (NULL != (nodemask = numa_allocate_nodemask())) else
    {
      runtime_error("numa_allocate_nodemask");
      return 2;
    }
  numa_bitmask_setbit(nodemask, node);
  numa_set_membind(nodemask);
  numa_free_nodemask(nodemask);

  return 0;
}
