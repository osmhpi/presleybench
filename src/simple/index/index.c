
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

int
index_init (struct index_t *index, enum index_type_e type)
{
  switch (type)
    {
      case INDEX_TYPE_BPLUS:
        index->prepare = &index_bplus_prepare;
        index->get = (index_get_func*)&bplus_tree_get;
        index->put = (index_put_func*)&bplus_tree_put;
        break;
      case INDEX_TYPE_GROUPKEY:
        index->prepare = (index_prepare_func*)&groupkey_prepare;
        index->get = (index_get_func*)&groupkey_get;
        index->put = (index_put_func*)&groupkey_put;
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
      case INDEX_TYPE_GROUPKEY:
        return "GroupKey";
      default:
        debug_error("unrecognized index type %i (%s)", arguments.index_type, arguments._index_type);
        return "unknown";
    }
}
