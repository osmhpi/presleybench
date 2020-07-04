#ifndef INDEX_H
#define INDEX_H

#include "simple/index/bplustree.h"

enum index_type_e
{
  INDEX_TYPE_BPLUS,
};

struct index_t
{
  // this union must remain the first member of the struct!
  union {
    struct bplus_tree bplus_tree;
  };

  enum index_type_e type;

  int(*prepare)(struct index_t*);
  int(*put)(struct index_t*, int, int);
  int(*get)(struct index_t*, int);
};

int index_init (struct index_t*, enum index_type_e);

const char *index_type_name (enum index_type_e);

#endif
