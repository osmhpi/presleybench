#ifndef DATA_H
#define DATA_H

#include "util/assert.h"
#include "simple/index/index.h"

#include <stddef.h>

extern size_t data_rows;
extern int *data_array;
extern size_t data_range;
extern struct index_t data_index;

int data_setup (size_t, size_t) att_warn_unused_result;

int data_linear_search (int*, size_t, int) att_warn_unused_result;

int data_index_search (struct index_t*, int) att_warn_unused_result;

#endif
