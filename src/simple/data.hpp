#ifndef DATA_H
#define DATA_H

#include "util/assert.hpp"
#include "simple/index/index.hpp"

#include <PGASUS/base/node.hpp>
#include <PGASUS/msource/msource_types.hpp>
#include <PGASUS/msource/msource.hpp>
#include <PGASUS/tasking/tasking.hpp>

#include <cstddef>
#include <vector>

extern size_t data_rows;
extern size_t data_range;
extern int *data_array;
extern std::vector<struct index_t*> data_index;
extern numa::Node primaryNode;

int data_setup (size_t, size_t) _warn_unused_result;

int data_linear_search (int*, size_t, int) _warn_unused_result;

int data_index_search (struct index_t*, int) _warn_unused_result;

#endif
