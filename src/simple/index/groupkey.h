#ifndef GROUPKEY_H
#define GROUPKEY_H

#include "util/assert.h"

#include <stddef.h>

struct groupkey_t
{
  struct {
    size_t n;
    size_t capacity;
    int *values;
    size_t *offsets;
  } unique;
  struct {
    size_t n;
    size_t capacity;
    size_t *postings;
  } index;
};

int groupkey_prepare (struct groupkey_t*) att_warn_unused_result;
int groupkey_put (struct groupkey_t*, int, int) att_warn_unused_result;
int groupkey_get (struct groupkey_t*, int) att_warn_unused_result;

#endif
