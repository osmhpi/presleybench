
#define _GNU_SOURCE

#include "simple/topology.h"
#include "simple/argparse.h"

#if HAVE_NUMA
#  include <numa.h>
#  include <sched.h>
#else
#  include <unistd.h>
#  if __APPLE__
#    include <pthread.h>
#    include <mach/thread_act.h>
#  endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct topology_t topology = { 0 };

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

#ifdef HAVE_NUMA
static int att_warn_unused_result
topology_node_of_cpu (int cpu)
{
  return numa_node_of_cpu(cpu);
}
#endif

static int att_warn_unused_result
topology_get_current_node (void)
{
#ifdef HAVE_NUMA
  return numa_node_of_cpu(sched_getcpu());
#else
  return 0;
#endif
}

static int att_warn_unused_result
topology_discover (void)
{
#ifdef HAVE_NUMA
  //fprintf(stderr, "numa_max_possible_node: %i\n", numa_max_possible_node());
  //fprintf(stderr, "numa_num_possible_nodes: %i\n", numa_num_possible_nodes());
  //fprintf(stderr, "numa_max_node: %i\n", numa_max_node());

  topology.nodes.n = numa_num_configured_nodes();
  //fprintf(stderr, "numa_num_configured_nodes: %zu\n", topology.nodes.n);
#else
  topology.nodes.n = 1;
#endif

  guard (NULL != (topology.nodes.nodes = malloc(sizeof(*topology.nodes.nodes) * topology.nodes.n))) else { return 2; }
  memset(topology.nodes.nodes, 0, sizeof(*topology.nodes.nodes) * topology.nodes.n);

#ifdef HAVE_NUMA
  struct bitmask *mask = numa_all_nodes_ptr;
  //fprintf(stderr, "mask size: %lu\n", mask->size);

  fprintf(stderr, "node mask: ");
  unsigned long i, n;
  for (i = 0, n = 0; i < mask->size; ++i)
    {
      if (i > 0 && i % 64 == 0)
        fprintf(stderr, "\n           ");

      int set = numa_bitmask_isbitset(mask, i);
      if (set)
        topology.nodes.nodes[n++].num = i;

      fprintf(stderr, "%i", set);
    }
  fprintf(stderr, "\n");
#else
  topology.nodes.nodes[0].num = 0;
#endif

#ifdef HAVE_NUMA
  int num_configured_cpus = numa_num_configured_cpus();
  fprintf(stderr, "numa_num_configured_cpus: %i\n", num_configured_cpus);

  mask = numa_all_cpus_ptr;
  //printf("mask size: %lu\n", mask->size);

  fprintf(stderr, "cpu mask : ");
  for (i = 0, n = 0; i < mask->size; ++i)
    {
      if (i > 0 && i % 64 == 0)
        fprintf(stderr, "\n           ");

      int set = numa_bitmask_isbitset(mask, i);
      if (set)
        {
          int nodenum;
          // this sometimes fails on very large systems (~400 cores)
          guard (0 <= (nodenum = topology_node_of_cpu(i))) else
            {
              fprintf(stderr, "X");
              continue;
            }
          struct node_t *node;
          guard (NULL != (node = topology_node_get(nodenum))) else { return 1; }
          int res;
          guard (0 == (res = node_add_cpu(node, i))) else { return res; }
        }

      fprintf(stderr, "%i", set);
    }
  fprintf(stderr, "\n");
#else
  int num_configured_cpus = sysconf(_SC_NPROCESSORS_CONF);
  fprintf(stderr, "_SC_NPROCESSORS_CONF: %i\n", num_configured_cpus);

  int i;
  for (i = 0; i < num_configured_cpus; ++i)
    {
      int res;
      guard (0 == (res = node_add_cpu(topology.nodes.nodes, i))) else { return res; }
    }
#endif

  return 0;
}

int
topology_setup (void)
{
  int res;
#ifdef HAVE_NUMA
  guard (0 == (res = numa_available())) else
    {
      runtime_error("failed to enable NUMA support");
      return res;
    }
  numa_set_strict(1);
#endif

  if (arguments.primary_node < 0)
    {
      arguments.primary_node = topology_get_current_node();
    }

  fprintf(stderr, "attempting to bind master task allocations to node #%i\n", arguments.primary_node);
#ifndef HAVE_NUMA
  fprintf(stderr, "  NOTE: NUMA support disabled at compile time\n");
#endif

  guard (0 == (res = topology_membind_to_node(arguments.primary_node))) else
    {
      runtime_error("failed to bind memory allocations to node #%i", arguments.primary_node);
      return res;
    }

  guard (0 == (res = topology_discover())) else { return res; }

  fprintf(stderr, "discovered topology information:\n");

  size_t i, n;
  for (i = 0; i < topology.nodes.n; ++i)
    {
      fprintf(stderr, "  node #%i\n", topology.nodes.nodes[i].num);
      fprintf(stderr, "  cpus:");
      for (n = 0; n < topology.nodes.nodes[i].cpus.n; ++n)
        {
          fprintf(stderr, " %i", topology.nodes.nodes[i].cpus.cpus[n]);
        }
      fprintf(stderr, "\n");
    }

  return 0;
}

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

int
topology_pin_to_node (int node)
{
#ifdef HAVE_NUMA
  int res;
  guard (0 == (res = numa_run_on_node(node))) else { return res; }
#else
  // noop -- if we have no NUMA support, the system is presented as a single node
  (void)node;
#endif

  return 0;
}

int
topology_pin_to_cpu (int cpu)
{
#if __linux__
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(cpu, &set);

  int res;
  guard (0 == (res = sched_setaffinity(0, sizeof(cpu_set_t), &set))) else { return res; }
#elif __APPLE__
  thread_affinity_policy_data_t policy_data = { cpu + 1 };
  int res;
  guard (0 == (res = thread_policy_set(pthread_mach_thread_np(pthread_self()), THREAD_AFFINITY_POLICY, (thread_policy_t)&policy_data, 1))) else { return res; }
#else
#  error no thread pinning available
#endif

  return 0;
}

int
topology_membind_to_node (int node)
{
#ifdef HAVE_NUMA
  struct bitmask *nodemask;
  guard (NULL != (nodemask = numa_allocate_nodemask())) else { return 2; }

  numa_bitmask_setbit(nodemask, node);
  numa_set_membind(nodemask);
  numa_free_nodemask(nodemask);
#else
  (void)node;
  // noop -- if we have no NUMA support, the system is presented as a single node
#endif

  return 0;
}

size_t
topology_cpu_count (void)
{
  size_t res = 0;

  size_t i;
  for (i = 0; i < topology.nodes.n; ++i)
    {
      res += topology.nodes.nodes[i].cpus.n;
    }

  return res;
}
