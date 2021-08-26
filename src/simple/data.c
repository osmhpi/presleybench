
#include "simple/data.h"
#include "simple/argparse.h"
#include "simple/topology.h"
#include "util/progress.h"

#if HAVE_NUMA
#  include <numa.h>
#else
#  define numa_alloc_onnode(S, N) malloc(S)
#  define numa_free(P, S) free(P)
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

size_t data_rows = 0;
size_t data_range = 0;
int *data_array = NULL;
struct index_t **data_index = NULL;
int primary_node = -1;

static int
populate_data_array (void)
{
  fprintf(stderr, "  generating values ...\n");

  if (arguments.primary_node != -1)
    primary_node = arguments.primary_node;
  else
    primary_node = numa_preferred();

  int *tmp_array = NULL;
  guard (NULL != (tmp_array = numa_alloc_onnode(sizeof(*data_array) * data_range, primary_node))) else { return 2; }

  // generate sorted array
  size_t i;
  PROGRESS_BEGIN(data_range);
  for (i = 0; i < data_range; ++i)
    {
      PROGRESS_UPDATE("    ", i);
      tmp_array[i] = i;
    }
  PROGRESS_FINISH("    ");

  fprintf(stderr, "  performing fisher-yates shuffle ...\n");

  // shuffle sorted array
  PROGRESS_BEGIN(data_range);
  for (i = data_range - 1; i > 0; --i)
    {
      PROGRESS_UPDATE("    ", data_range - i - 1);
      int j = rand() % i;
      int tmp = tmp_array[j];
      tmp_array[j] = tmp_array[i];
      tmp_array[i] = tmp;
    }
  PROGRESS_FINISH("    ");

  // truncate to size by copying (don't trust numa_realloc)
  guard (NULL != (data_array = numa_alloc_onnode(sizeof(*data_array) * data_rows, primary_node))) else { return 2; }

  fprintf(stderr, "  truncating to final size ...\n");

  PROGRESS_BEGIN(data_rows);
  for (i = 0; i < data_rows; ++i)
    {
      PROGRESS_UPDATE("    ", i);
      data_array[i] = tmp_array[i];
    }
  PROGRESS_FINISH("    ");

  // tidy up
  numa_free(tmp_array, sizeof(*data_array) * data_range);

#if DEBUG
  // write generated data array to disk
  FILE *out = NULL;
  guard (NULL != (out = fopen("data_array.txt", "w"))) else { return 3; }

  for (i = 0; i < data_rows; ++i)
    {
      fprintf(out, "%i\n", data_array[i]);
    }

  int res;
  guard (0 == (res = fclose(out))) else { return res; }
#endif

  return 0;
}

static int
populate_data_index (struct index_t *index)
{
  int res;

  // prepare index
  guard (0 == (res = index_init(index, arguments.index_type))) else { return res; }
  guard (0 == (res = index->prepare(index))) else { return res; }

  // populate index
  fprintf(stderr, "  populating values ...\n");

  size_t i;
  PROGRESS_BEGIN(data_rows);
  for (i = 0; i < data_rows; ++i)
    {
      PROGRESS_UPDATE("    ", i);
      guard (0 == (res = index->put(index, data_array[i], i))) else { return res; }
    }
  PROGRESS_FINISH("    ");

  return 0;
}

int
data_setup (size_t rows, size_t range)
{
  int res;

  data_range = range;
  data_rows = rows;

  // don't populate more rows than we have possible values
  guard (data_rows <= data_range) else { errno = EINVAL; return 1; }

  fprintf(stderr, "preparing data array ...\n");
  guard (0 == (res = populate_data_array())) else { return res; }
  fprintf(stderr, "  data size: %zu Bytes\n", rows * sizeof(*data_array));

  // finish here, if we don't need to populate an index
  if (!arguments.index_search)
    return 0;

  guard (NULL != (data_index = malloc(sizeof(*data_index) * (numa_max_node() + 1)))) else { return 2; }
  memset(data_index, 0, sizeof(*data_index) * (numa_max_node() + 1));

  if (!arguments.replicate)
    {
      fprintf(stderr, "preparing %s index ...\n", index_type_name(arguments.index_type));

      struct index_t *index = numa_alloc_onnode(sizeof(*index), primary_node);
      memset(index, 0, sizeof(*index));

      guard (0 == (res = populate_data_index(index))) else { return res; }
      data_index[primary_node] = index;
    }
  else
    {
      size_t i;
      for (i = 0; i < topology.nodes.n; ++i)
        {
          fprintf(stderr, "preparing %s index replica on node #%i ...\n", index_type_name(arguments.index_type), topology.nodes.nodes[i].num);

          struct index_t *index = numa_alloc_onnode(sizeof(*index), topology.nodes.nodes[i].num);
          memset(index, 0, sizeof(*index));

          guard (0 == (res = populate_data_index(index))) else { return res; }
          data_index[topology.nodes.nodes[i].num] = index;
        }
    }

  return 0;
}

int
data_linear_search (int *array, size_t rows, int needle)
{
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
  return index->get(index, needle);
}
