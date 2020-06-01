#ifndef DATA_H
#define DATA_H

#include "util/assert.h"
#include "simple/bplustree.h"

#include <stddef.h>

extern size_t data_rows;
extern int *data_array;
extern size_t data_range;
extern struct bplus_tree *data_tree;
extern struct bplus_tree **replica_trees;

int data_setup (size_t, size_t) att_warn_unused_result;

int data_linear_search (int*, size_t, int) att_warn_unused_result;

int data_tree_search (struct bplus_tree*, int) att_warn_unused_result;

#endif
