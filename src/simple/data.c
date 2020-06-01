
#include "simple/data.h"
#include "simple/bplustree.h"
#include "simple/argparse.h"

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

  printf("preparing data array ...\n");
  printf("  initializing values ...\n");

  guard (NULL != (data_array = malloc(sizeof(*data_array) * range))) else { return 2; }

  int increment = range / 1000;
  size_t i;
  for (i = 0; i < range; ++i)
    {
      if (!(i % increment))
        {
          int progress = i / increment;
          printf("\r    %c  %i.%i %%", "-\\|/"[progress % 4], progress / 10, progress % 10);
          fflush(stdout);
        }
      data_array[i] = i;
    }
  printf("\r    *  100 %% \n");

  printf("  performing fisher-yates shuffle ...\n");

  for (i = range - 1; i > 0; --i)
    {
      if (!((range - i - 1) % increment))
        {
          int progress = (range - i - 1) / increment;
          printf("\r    %c  %i.%i %%", "-\\|/"[progress % 4], progress / 10, progress % 10);
          fflush(stdout);
        }
      int j = rand() % i;
      int tmp = data_array[j];
      data_array[j] = data_array[i];
      data_array[i] = tmp;
    }
  printf("\r    *  100 %% \n");

  // truncate to SIZE
  guard (NULL != (data_array = realloc(data_array, sizeof(*data_array) * rows))) else { return 2; }

  if (!arguments.tree_search)
    return 0;

  printf("preparing B+ tree ...\n");
  printf("  populating values ...\n");

  // populate tree
  guard (NULL != (data_tree = bplus_tree_init(TREE_ORDER, TREE_ENTRIES))) else { return 2; }

  increment = rows / 1000;

  for (i = 0; i < rows; ++i)
    {
      if (!(i % increment))
        {
          int progress = i / increment;
          printf("\r    %c  %i.%i %%", "-\\|/"[progress % 4], progress / 10, progress % 10);
          fflush(stdout);
        }
      bplus_tree_put(data_tree, data_array[i], i);
    }
  printf("\r    *  100 %% \n");

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
