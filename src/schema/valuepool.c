
#include "schema/valuepool.h"

#include "util/assert.h"

#include <errno.h>

int
valuepool_init (struct valuepool_t *pool, enum valuepool_e type)
{
  switch (type)
    {
      case VALUEPOOL_NONE:
        break;
      case VALUEPOOL_CAPACITY:
        pool->capacity = 0;
        break;
      case VALUEPOOL_REFERENCE:
        pool->ref = NULL;
        break;
      case VALUEPOOL_STRINGS:
        list_init(&pool->_strings);
        break;
      default:
        debug_error("unexpected value for 'type': '%i'", type);
        errno = EINVAL;
        return 1;
    }

  pool->type = type;
  pool->column = NULL;

  return 0;
}

void
valuepool_fini (struct valuepool_t *pool)
{
  switch(pool->type)
    {
      case VALUEPOOL_NONE:
        break;
      case VALUEPOOL_CAPACITY:
        pool->capacity = 0;
        break;
      case VALUEPOOL_REFERENCE:
        pool->ref = NULL;
        break;
      case VALUEPOOL_STRINGS:
        list_fini(&pool->_strings, NULL);
        break;
      default:
        debug_error("pool is corrupted. this may leak memory! 'type': '%i'", pool->type);
        break;
    }

  pool->type = VALUEPOOL_NONE;
  pool->column = NULL;
}
