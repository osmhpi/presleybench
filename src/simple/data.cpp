
#include "simple/data.hpp"
#include "simple/argparse.hpp"
#include "util/progress.hpp"

#include <PGASUS/base/node.hpp>
#include <PGASUS/msource/msource_types.hpp>
#include <PGASUS/msource/msource.hpp>
#include <PGASUS/tasking/tasking.hpp>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

size_t data_rows = 0;
size_t data_range = 0;
int *data_array = NULL;
std::vector<struct index_t*> data_index;
numa::Node primaryNode;

static int
populate_data_array (void)
{
  std::cerr << "  generating values ..." << std::endl;

  if (arguments.primary_node != -1)
    primaryNode = numa::NodeList::logicalNodes()[numa::NodeList::physicalToLogicalId(arguments.primary_node)];
  else
    primaryNode = numa::Node::curr();

  numa::PlaceGuard mguard(primaryNode);

  int *tmp_array = NULL;
  guard (NULL != (tmp_array = new int[data_range])) else { return 2; }

  // generate sorted array
  size_t i;
  PROGRESS_BEGIN(data_range);
  for (i = 0; i < data_range; ++i)
    {
      PROGRESS_UPDATE("    ", i);
      tmp_array[i] = i;
    }
  PROGRESS_FINISH("    ");

  std::cerr << "  performing fisher-yates shuffle ..." << std::endl;

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
  guard (NULL != (data_array = new int[data_rows])) else { return 2; }

  std::cerr << "  truncating to final size ..." << std::endl;

  PROGRESS_BEGIN(data_rows);
  for (i = 0; i < data_rows; ++i)
    {
      PROGRESS_UPDATE("    ", i);
      data_array[i] = tmp_array[i];
    }
  PROGRESS_FINISH("    ");

  // tidy up
  delete tmp_array;

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
  std::cerr << "  populating values ..." << std::endl;

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

  std::cerr << "preparing data array ..." << std::endl;
  guard (0 == (res = populate_data_array())) else { return res; }
  std::cerr << "  data size: " << rows * sizeof(*data_array) << " Bytes" << std::endl;

  // finish here, if we don't need to populate an index
  if (!arguments.index_search)
    return 0;

  for (numa::Node _ : numa::NodeList::logicalNodesWithCPUs())
    {
      data_index.push_back(NULL);
    }

  if (!arguments.replicate)
    {
      std::cerr << "preparing " << index_type_name(arguments.index_type) << " index ..." << std::endl;

      numa::PlaceGuard mguard(primaryNode);

      struct index_t *index = new struct index_t;
      memset(index, 0, sizeof(*index));

      guard (0 == (res = populate_data_index(index))) else { return res; }
      data_index[primaryNode.logicalId()] = index;
    }
  else
    {
      numa::wait(numa::onceForEachNode(numa::NodeList::logicalNodesWithCPUs(), [&]()
        {
          std::cerr << "preparing " << index_type_name(arguments.index_type)
              << " index replica on node #" << numa::Node::curr().logicalId() << " ..." << std::endl;

          numa::Node current = numa::Node::curr();
          numa::PlaceGuard mguard(current);

          struct index_t *index = new struct index_t;
          memset(index, 0, sizeof(*index));

          guard (0 == (res = populate_data_index(index))) else { return res; }
          data_index[current.logicalId()] = index;

          return 0;
        }, 0));
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
