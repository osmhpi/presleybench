
#include "simple/data.h"
#include "simple/argparse.h"
#include "simple/topology.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef HAVE___LIBC_CALLOC
#  include <dlfcn.h>
#endif

#define TREE_ORDER 64
#define TREE_ENTRIES 64

static int aggregation_enabled = 0;
static size_t bytes = 0;

#ifdef HAVE___LIBC_CALLOC
extern void* __libc_calloc(size_t, size_t);
#endif

void*
calloc (size_t nmemb, size_t size)
{
#ifndef HAVE___LIBC_CALLOC
  static void*(*__libc_calloc)(size_t, size_t) = NULL;
  if (__libc_calloc == NULL)
    {
      guard (NULL != (__libc_calloc = dlsym(RTLD_NEXT, "calloc"))) else { return NULL; }
    }
#endif

  if (aggregation_enabled)
    bytes += nmemb * size;

  return __libc_calloc(nmemb, size);
}

size_t data_rows = 0;
int *data_array = NULL;
size_t data_range = 0;
struct index_t *data_index = NULL;

int
data_setup (size_t rows, size_t range)
{
  data_range = range;
  data_rows = rows;

  guard (data_rows <= data_range) else { errno = EINVAL; return 1; }

  fprintf(stderr, "preparing data array ...\n");
  fprintf(stderr, "  initializing values ...\n");

  guard (NULL != (data_array = malloc(sizeof(*data_array) * range))) else { return 2; }

  int increment = range / 1000;
  if (increment == 0)
    increment = 1;
  size_t i;
  for (i = 0; i < range; ++i)
    {
      if (!(i % increment))
        {
          float progress = i / (float)range;
          fprintf(stderr, "\r    %c  %.1f %%", "-\\|/"[(i / increment) % 4], progress * 100.0);
          fflush(stdout);
        }
      data_array[i] = i;
    }
  fprintf(stderr, "\r    *  100 %% \n");

  fprintf(stderr, "  performing fisher-yates shuffle ...\n");

  for (i = range - 1; i > 0; --i)
    {
      if (!((range - i - 1) % increment))
        {
          float progress = (range - i - 1) / (float)range;
          fprintf(stderr, "\r    %c  %.1f %%", "-\\|/"[(i / increment) % 4], progress * 100.0);
          fflush(stdout);
        }
      int j = rand() % i;
      int tmp = data_array[j];
      data_array[j] = data_array[i];
      data_array[i] = tmp;
    }
  fprintf(stderr, "\r    *  100 %% \n");
  fprintf(stderr, "  data size: %zu Bytes\n", rows * sizeof(*data_array));

  // truncate to SIZE
  guard (NULL != (data_array = realloc(data_array, sizeof(*data_array) * rows))) else { return 2; }

  if (!arguments.tree_search)
    return 0;

  fprintf(stderr, "preparing B+ tree ...\n");
  fprintf(stderr, "  populating values ...\n");

  // populate tree
  aggregation_enabled = 1;
  guard (NULL != (data_index = (struct index_t*)bplus_tree_init(TREE_ORDER, TREE_ENTRIES))) else { return 2; }
  aggregation_enabled = 0;

  increment = rows / 1000;
  if (increment == 0)
    increment = 1;
  for (i = 0; i < rows; ++i)
    {
      if (!(i % increment))
        {
          float progress = i / (float)rows;
          fprintf(stderr, "\r    %c  %.1f %%", "-\\|/"[(i / increment) % 4], progress * 100.0);
          fflush(stdout);
        }
      aggregation_enabled = 1;
      bplus_tree_put((struct bplus_tree*)data_index, data_array[i], i);
      aggregation_enabled = 0;
    }

  fprintf(stderr, "\r    *  100 %% \n");
  fprintf(stderr, "  tree size: %zu Bytes\n", bytes);

  if (!arguments.replicate)
    return 0;

  fprintf(stderr, "preparing B+ tree replicates ...\n");

  for (i = 0; i < topology.nodes.n; ++i)
    {
      topology.nodes.nodes[i].replica = NULL;
      fprintf(stderr, "  populating replica for node #%i ...\n", topology.nodes.nodes[i].num);

      int res;
      guard (0 == (res = topology_membind_to_node(topology.nodes.nodes[i].num))) else
        {
          runtime_error("failed to bind memory allocations to node #%i", arguments.primary_node);
          return res;
        }

      guard (NULL != (topology.nodes.nodes[i].replica = (struct index_t*)bplus_tree_init(TREE_ORDER, TREE_ENTRIES))) else { return 2; }

      size_t j;
      for (j = 0; j < rows; ++j)
        {
          if (!(j % increment))
            {
              float progress = i / (float)rows;
              fprintf(stderr, "\r    %c  %.1f %%", "-\\|/"[(i / increment) % 4], progress * 100.0);
              fflush(stdout);
            }
          bplus_tree_put((struct bplus_tree*)topology.nodes.nodes[i].replica, data_array[j], j);
        }
      fprintf(stderr, "\r    *  100 %% \n");
    }

  int res;
  guard (0 == (res = topology_membind_to_node(arguments.primary_node))) else
    {
      runtime_error("failed to bind memory allocations to node #%i", arguments.primary_node);
      return res;
    }

  return 0;
}

int
data_linear_search (int *array, size_t rows, int needle)
{
  // linear search
  size_t i;
  for (i = 0; i < rows; ++i)
    {
      if (array[i] == needle)
        return (int)i;
    }

  return -1;
}

int
data_index_search (struct index_t *index, int needle)
{
  return bplus_tree_get((struct bplus_tree*)index, needle);
}
