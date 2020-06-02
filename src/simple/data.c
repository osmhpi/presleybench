
#include "simple/data.h"
#include "simple/bplustree.h"
#include "simple/argparse.h"
#include "simple/topology.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define TREE_ORDER 64
#define TREE_ENTRIES 64


size_t data_rows = 0;
int *data_array = NULL;
size_t data_range = 0;
struct bplus_tree *data_tree = NULL;

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
  size_t i;
  for (i = 0; i < range; ++i)
    {
      if (!(i % increment))
        {
          int progress = i / increment;
          fprintf(stderr, "\r    %c  %i.%i %%", "-\\|/"[progress % 4], progress / 10, progress % 10);
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
          int progress = (range - i - 1) / increment;
          fprintf(stderr, "\r    %c  %i.%i %%", "-\\|/"[progress % 4], progress / 10, progress % 10);
          fflush(stdout);
        }
      int j = rand() % i;
      int tmp = data_array[j];
      data_array[j] = data_array[i];
      data_array[i] = tmp;
    }
  fprintf(stderr, "\r    *  100 %% \n");

  // truncate to SIZE
  guard (NULL != (data_array = realloc(data_array, sizeof(*data_array) * rows))) else { return 2; }

  if (!arguments.tree_search)
    return 0;

  fprintf(stderr, "preparing B+ tree ...\n");
  fprintf(stderr, "  populating values ...\n");

  // populate tree
  guard (NULL != (data_tree = bplus_tree_init(TREE_ORDER, TREE_ENTRIES))) else { return 2; }

  increment = rows / 1000;

  for (i = 0; i < rows; ++i)
    {
      if (!(i % increment))
        {
          int progress = i / increment;
          fprintf(stderr, "\r    %c  %i.%i %%", "-\\|/"[progress % 4], progress / 10, progress % 10);
          fflush(stdout);
        }
      bplus_tree_put(data_tree, data_array[i], i);
    }
  fprintf(stderr, "\r    *  100 %% \n");

  if (!arguments.replicate)
    return 0;

  fprintf(stderr, "preparing B+ tree replicates ...\n");

  for (i = 0; i < topology.nodes.n; ++i)
    {
      topology.nodes.nodes[i].replica = NULL;
      fprintf(stderr, "  populating replica for node #%i ...\n", topology.nodes.nodes[i].num);

      int res;
      guard (0 == (res = numa_membind_to_node(topology.nodes.nodes[i].num))) else
        {
          runtime_error("failed to bind memory allocations to node #%i", arguments.primary_node);
          return res;
        }

      guard (NULL != (topology.nodes.nodes[i].replica = bplus_tree_init(TREE_ORDER, TREE_ENTRIES))) else { return 2; }

      size_t j;
      for (j = 0; j < rows; ++j)
        {
          if (!(j % increment))
            {
              int progress = j / increment;
              fprintf(stderr, "\r    %c  %i.%i %%", "-\\|/"[progress % 4], progress / 10, progress % 10);
              fflush(stdout);
            }
          bplus_tree_put(topology.nodes.nodes[i].replica, data_array[j], j);
        }
      fprintf(stderr, "\r    *  100 %% \n");
    }

  int res;
  guard (0 == (res = numa_membind_to_node(arguments.primary_node))) else
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
data_tree_search (struct bplus_tree *tree, int needle)
{
  return bplus_tree_get(tree, needle);
}
