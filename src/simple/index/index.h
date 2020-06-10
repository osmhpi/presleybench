#ifndef INDEX_H
#define INDEX_H

#include "simple/index/bplustree.h"

struct index_t
{
  // this union must remain the first member of the struct!
  union {
    struct bplus_tree bplus_tree;
  };
};

#endif
