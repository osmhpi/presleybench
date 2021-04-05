
#include "simple/index/groupkey.h"

// excerpt from https://github.com/hyrise/hyrise/wiki/GroupKey:
//
// The GroupKeyIndex works on a single dictionary compressed column. It uses
// two structures:
//
//     A postings-list containing record positions (i.e. ChunkOffsets) in the
//     attribute vector and An index offset, mapping value-IDs to offsets in
//     the postings-list
//
// An example structure along with the corresponding dictionary column might
// look like this:
//   (i)  Attribute Vector  Dictionary  Index Offset  Index Postings
//    0    4                 apple       0             4
//    1    2                 charlie     1             5
//    2    3                 delta       3             6
//    3    2                 frank       5             1
//    4    0                 hotel       6             3
//    5    1                 inbox       7             2
//    6    1                                           0
//    7    5                                           7
//
// How to read this:
//
//     The 0th-row means: The position for the value "apple" (with the valueID
//     0) can be found at the 0th, i.e. first position, in the index postings.
//     I.e. "apple" can be found at i = 4 in the attribute vector.
//
//     The 2nd (until 4th) row means: The position for the value "delta" (with
//     the valueID 3) can be found from i = 3 until i = 5 in the index
//     postings. I.e. "delta" can be found at i =1 and 3 in the attribute
//     vector.
//
//     The 5th (until 7th) row means: The position for the value "inbox" (with
//     the valueID 5) can be found from i = 7 up to the end in the index
//     postings. I.e. "inbox" can be found at i = 7 in the attribute vector.
//
// This is also how the index looks up values internally: Given a search value,
// the corresponding valueID is retrieved from the underlying
// DictionarySegment. Then the value in the index offset at the position =
// valueID is looked up. This value tells us where in the index postings we
// start reading the resulting ChunkOffsets. To know where to stop reading, we
// search for the value in the index offset at the position = valueID + 1, i.e.
// the upper-bound of the search value.
//
// In Hyrise, this means we first request the lower-bound on the index for a
// certain search value, we thus retrieve the iterator pointing to the
// corresponding position in the index postings, and iterate forward until we
// reach the upper-bound on the index for this search value.

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define CAPACITY_INCREMENT (4 * 1024 / sizeof(size_t)) // number of size_t's in a page of memory

int
groupkey_prepare (struct groupkey_t *key)
{
  key->unique.n = 0;
  key->unique.capacity = 0;
  key->unique.values = NULL;
  key->unique.offsets = NULL;

  key->index.n = 0;
  key->index.capacity = 0;
  key->index.postings = NULL;

  return 0;
}

static size_t
binary_search (size_t n, int *v, int k)
{
  // this binary search produces the index of the first element in v that is
  // greater than or equal to k. In other words, the index at which k would need
  // to be inserted to v such that v is still ordered.
  //
  // v is assumed to be ordered ascendingly.

  size_t l = 0;
  size_t r = n;

  while (l < r)
    {
      size_t m = (l + r) / 2;

      if (v[m] < k)
        l = m + 1;
      else
        r = m;
    }

  return l;
}

int
groupkey_get (struct groupkey_t *key, int k)
{
  size_t value_pos = binary_search(key->unique.n, key->unique.values, k);
  if (key->unique.values[value_pos] != k)
    return -1; // not found

  // this would give us the complete range of matching values.
  //size_t offsets_begin = key->unique.offsets[value_pos];
  //size_t offsets_end = (value_pos == key->unique.n - 1) ? key->unique.n : key->unique.offsets[value_pos + 1];

  // we only need any match, so the first one is good enough.
  return key->index.postings[key->unique.offsets[value_pos]];
}

static int
value_insert (struct groupkey_t *key, int k, size_t pos)
{
  if (key->unique.n >= key->unique.capacity)
    {
      size_t new_capacity = key->unique.capacity + CAPACITY_INCREMENT;

      int *tv;
      guard (NULL != (tv = realloc(key->unique.values, sizeof(*key->unique.values) * new_capacity))) else { return 2; }
      key->unique.values = tv;

      size_t *to;
      guard (NULL != (to = realloc(key->unique.offsets, sizeof(*key->unique.offsets) * new_capacity))) else { return 2; }
      key->unique.offsets = to;

      key->unique.capacity = new_capacity;
    }

  memmove(key->unique.values + pos + 1, key->unique.values + pos, sizeof(*key->unique.values) * (key->unique.n - pos));
  memmove(key->unique.offsets + pos + 1, key->unique.offsets + pos, sizeof(*key->unique.offsets) * (key->unique.n - pos));

  key->unique.values[pos] = k;
  key->unique.offsets[pos] = (pos < key->unique.n) ? key->unique.offsets[pos + 1] : key->index.n;

  key->unique.n++;

  return 0;
}

static int
posting_insert (struct groupkey_t *key, int v, size_t value_pos)
{
  if (key->index.n >= key->index.capacity)
    {
      size_t new_capacity = key->index.capacity + CAPACITY_INCREMENT;

      size_t *tp;
      guard (NULL != (tp = realloc(key->index.postings, sizeof(*key->index.postings) * new_capacity))) else { return 2; }
      key->index.postings = tp;

      key->index.capacity = new_capacity;
    }

  size_t posting_pos_l = key->unique.offsets[value_pos];
  size_t posting_pos_r = (value_pos < key->unique.n) ? key->unique.offsets[value_pos + 1] : key->index.n;

  while (posting_pos_l < posting_pos_r)
    {
      size_t posting_pos_m = (posting_pos_l + posting_pos_r) / 2;

      if (key->index.postings[posting_pos_m] < (size_t)v)
        posting_pos_l = posting_pos_m + 1;
      else
        posting_pos_r = posting_pos_m;
    }

  size_t posting_pos = posting_pos_l;

  memmove(key->index.postings + posting_pos + 1, key->index.postings + posting_pos,
          sizeof(*key->index.postings) * (key->index.n - posting_pos));

  size_t i;
  for (i = value_pos + 1; i < key->unique.n; ++i)
    key->unique.offsets[i]++;

  key->index.postings[posting_pos] = v;
  key->index.n++;

  return 0;
}

int
groupkey_placement_put (struct groupkey_t *key, void **tail, int k, int v)
{
  //size_t value_pos = binary_search(key->unique.n, key->unique.values, k);
  //if (key->unique.n == 0 || key->unique.values[value_pos] != k)
  //  {
  //    int res;
  //    guard (0 == (res = value_insert(key, k, value_pos))) else { return res; }
  //  }

  //int res;
  //guard (0 == (res = posting_insert(key, v, value_pos))) else { return res; }
  
  // TODO
  errno = ENOSYS;
  return -1;
}

int
groupkey_put (struct groupkey_t *key, int k, int v)
{
  size_t value_pos = binary_search(key->unique.n, key->unique.values, k);
  if (key->unique.n == 0 || key->unique.values[value_pos] != k)
    {
      int res;
      guard (0 == (res = value_insert(key, k, value_pos))) else { return res; }
    }

  int res;
  guard (0 == (res = posting_insert(key, v, value_pos))) else { return res; }

  return 0;
}
