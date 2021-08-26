#ifndef INDEX_H
#define INDEX_H

#include "simple/index/bplustree.h"
#include "simple/index/groupkey.h"

enum index_type_e
{
  INDEX_TYPE_BPLUS,
  INDEX_TYPE_GROUPKEY,
};

struct index_t;

typedef int(index_prepare_func)(struct index_t*);
typedef void(index_deinit_func)(struct index_t*);
typedef int(index_put_func)(struct index_t*, int, int);
typedef int(index_get_func)(struct index_t*, int);

struct index_t
{
  // this union must remain the first member of the struct!
  union {
    struct bplus_tree bplus_tree;
    struct groupkey_t groupkey;
  };

  enum index_type_e type;

  index_prepare_func *prepare;
  index_deinit_func *deinit;
  index_put_func *put;
  index_get_func *get;
};

int index_init (struct index_t*, enum index_type_e);

void index_destroy (struct index_t*);

const char *index_type_name (enum index_type_e);

#endif
