
#include "simple/data.h"
#include "simple/argparse.h"
#include "simple/topology.h"
#include "simple/memory_aggregator.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

size_t data_rows = 0;
int *data_array = NULL;
size_t data_range = 0;
struct index_t data_index = { 0 };

static int progress_max;
static int progress_step;

#define PROGRESS_BEGIN(R) do { \
    progress_max = (R); \
    progress_step = (R) / 1000; \
    if (progress_step <= 0) \
      progress_step = 1; \
  } while (0)

#define PROGRESS_UPDATE(S, P) do { \
    if (!((P) % progress_step)) { \
      float progress = (P) / (float)progress_max; \
      fprintf(stderr, "\r" S "%c  %.1f %%", "-\\|/"[(i / progress_step) % 4], progress * 100.0); \
    } \
  } while (0)

#define PROGRESS_FINISH(S) do { \
    fprintf(stderr, "\r" S "*  100.0 %% \n"); \
  } while (0)

static int
populate_data_array (void)
{
  fprintf(stderr, "  generating values ...\n");

  guard (NULL != (data_array = malloc(sizeof(*data_array) * data_range))) else { return 2; }

  size_t i;
  PROGRESS_BEGIN(data_range);
  for (i = 0; i < data_range; ++i)
    {
      PROGRESS_UPDATE("    ", i);

      data_array[i] = i;
    }
  PROGRESS_FINISH("    ");

  fprintf(stderr, "  performing fisher-yates shuffle ...\n");

  PROGRESS_BEGIN(data_range);
  for (i = data_range - 1; i > 0; --i)
    {
      PROGRESS_UPDATE("    ", data_range - i - 1);

      int j = rand() % i;
      int tmp = data_array[j];
      data_array[j] = data_array[i];
      data_array[i] = tmp;
    }
  PROGRESS_FINISH("    ");

  // truncate to SIZE
  guard (NULL != (data_array = realloc(data_array, sizeof(*data_array) * data_rows))) else { return 2; }

  return 0;
}

static int
populate_data_index (struct index_t *index)
{
  int res;

  // prepare allocation aggregator
  guard (0 == (res = aggregator_clear())) else { return res; }

  // prepare index
  aggregator_enabled = 1;
  guard (0 == (res = index_init(index, arguments.index_type))) else { return res; }
  guard (0 == (res = index->prepare(index))) else { return res; }
  aggregator_enabled = 0;

  // populate index
  fprintf(stderr, "  populating values ...\n");

  size_t i;
  PROGRESS_BEGIN(data_rows);
  for (i = 0; i < data_rows; ++i)
    {
      PROGRESS_UPDATE("    ", i);

      aggregator_enabled = 1;
      guard (0 == (res = index->put(index, data_array[i], i))) else { return res; }
      aggregator_enabled = 0;
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

  fprintf(stderr, "preparing %s index ...\n", index_type_name(arguments.index_type));
  guard (0 == (res = populate_data_index(&data_index))) else { return res; }
  fprintf(stderr, "  index size: %zu Bytes\n", aggregator_bytes);

  // finish here, if we don't need to replicate the index
  if (!arguments.replicate)
    return 0;

  size_t i;
  for (i = 0; i < topology.nodes.n; ++i)
    {
      fprintf(stderr, "preparing %s index replica on node #%i ...\n", index_type_name(arguments.index_type), topology.nodes.nodes[i].num);

      guard (0 == (res = topology_membind_to_node(topology.nodes.nodes[i].num))) else { return res; }
      guard (0 == (res = populate_data_index(&topology.nodes.nodes[i].data_index))) else { return res; }
    }

  guard (0 == (res = topology_membind_to_node(arguments.primary_node))) else { return res; }

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
  return data_index.get(index, needle);
}
