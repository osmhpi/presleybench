
#include "simple/index/index.h"

#include "simple/argparse.h"
#include "util/assert.h"

static int
index_bplus_prepare (struct index_t *index)
{
  static const int bplus_tree_order = 64;
  static const int bplus_tree_entries = 64;

  return bplus_tree_initi((struct bplus_tree*)index, bplus_tree_order, bplus_tree_entries);
}

static int
index_bplus_put (struct index_t *index, int key, int value)
{
  // 0 is a valid data value for our use case, but the bplus implementation chokes on it
  int data = value + 1;
  return bplus_tree_put((struct bplus_tree*)index, key, data);
}

static int
index_bplus_get (struct index_t *index, int key)
{
  int res = bplus_tree_get((struct bplus_tree*)index, key);
  // reverse our value modification in bplus_put
  return (res > 0 ? res - 1 : res);
}

int
index_init (struct index_t *index, enum index_type_e type)
{
  switch (type)
    {
      case INDEX_TYPE_BPLUS:
        index->prepare = &index_bplus_prepare;
        index->get = &index_bplus_get;
        index->put = &index_bplus_put;
        break;
      default:
        runtime_error("unrecognized index type %i (%s)", arguments.index_type, arguments._index_type);
        return 1;
    }

  index->type = type;

  return 0;
}

const char*
index_type_name (enum index_type_e type)
{
  switch (type)
    {
      case INDEX_TYPE_BPLUS:
        return "B+ Tree";
      default:
        debug_error("unrecognized index type %i (%s)", arguments.index_type, arguments._index_type);
        return "unknown";
    }
}
