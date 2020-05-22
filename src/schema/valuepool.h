#ifndef VALUEPOOL_H
#define VALUEPOOL_H

#include "util/list.h"

enum valuepool_e
{
  VALUEPOOL_NONE,
  VALUEPOOL_CAPACITY,
  VALUEPOOL_REFERENCE,
  VALUEPOOL_STRINGS,
};

struct valuepool_t
{
  enum valuepool_e type;

  union {
    size_t capacity;
    struct valuepool_t *ref;
    union {
      struct list_t _strings;
      struct {
        size_t n;
        char **strings;
      } strings;
    };
  };
};

int valuepool_init (struct valuepool_t*, enum valuepool_e);

void valuepool_fini (struct valuepool_t*);

#endif
