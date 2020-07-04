
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "simple/memory_aggregator.h"
#include "util/assert.h"

#include <stdlib.h>
#include <string.h>

#if !defined(HAVE___LIBC_CALLOC) || \
    !defined(HAVE___LIBC_MALLOC) || \
    !defined(HAVE___LIBC_REALLOC) || \
    !defined(HAVE___LIBC_FREE)
#  include <dlfcn.h>
#endif

#ifdef HAVE___LIBC_MALLOC
extern void* __libc_malloc(size_t);
#else
static void*(*__libc_malloc)(size_t) = NULL;
#endif

#ifdef HAVE___LIBC_CALLOC
extern void* __libc_calloc(size_t, size_t);
#else
static void*(*__libc_calloc)(size_t, size_t) = NULL;
#endif

#ifdef HAVE___LIBC_REALLOC
extern void* __libc_realloc(void*, size_t);
#else
static void*(*__libc_realloc)(void*, size_t) = NULL;
#endif

#ifdef HAVE___LIBC_FREE
extern void* __libc_free(void*);
#else
static void*(*__libc_free)(void*) = NULL;
#endif

struct blocks_t
{
  size_t n;
  size_t c;
  struct {
    void *ptr;
    size_t size;
  } *e;
};

#define CAPACITY_INITIAL (4096 / sizeof(*blocks.e)) // page size
#define CAPACITY_INCREMENT CAPACITY_INITIAL

static struct blocks_t blocks = { 0, 0, NULL };

int aggregator_enabled;
size_t aggregator_bytes;

static void track_alloc (size_t, void*);
static void track_realloc (void*, size_t, void*);
static void track_free (void*);


int
aggregator_clear (void)
{
  free(blocks.e);
  blocks.e = NULL;
  blocks.n = 0;

  blocks.c = CAPACITY_INITIAL;
  guard (NULL != (blocks.e = malloc(sizeof(*blocks.e) * blocks.c))) else { return 2; }

  return 0;
}

static void
track_alloc (size_t size, void *res)
{
  if (!blocks.e)
    {
      int res;
      guard (0 == (res = aggregator_clear())) else { return; }
    }

  if (blocks.n >= blocks.c)
    {
#ifndef HAVE___LIBC_REALLOC
      if (__libc_realloc == NULL)
        {
          guard (NULL != (__libc_realloc = dlsym(RTLD_NEXT, "realloc"))) else { return; }
        }
#endif
      blocks.c += CAPACITY_INCREMENT;
      guard (NULL != (blocks.e = __libc_realloc(blocks.e, sizeof(*blocks.e) * blocks.c))) else { return; }
    }

  aggregator_bytes += size;
  blocks.e[blocks.n].ptr = res;
  blocks.e[blocks.n].size = size;
  blocks.n++;
}

static void
track_realloc (void *ptr, size_t size, void *res)
{
  if (!blocks.e)
    {
      int res;
      guard (0 == (res = aggregator_clear())) else { return; }
    }

  if (!ptr)
    return track_alloc(size, res);

  size_t i;
  for (i = 0; i < blocks.n; ++i)
    {
      if (blocks.e[i].ptr == ptr)
        {
          aggregator_bytes += size - blocks.e[i].size;
          blocks.e[i].ptr = res;
          blocks.e[i].size = size;
          return;
        }
    }

  debug_error("%p: unable to track ptr in call to realloc. bad ptr?", ptr);
  track_alloc(size, res);
}

static void
track_free (void* ptr)
{
  if (!blocks.e)
    {
      int res;
      guard (0 == (res = aggregator_clear())) else { return; }
    }

  if (!ptr)
    return;

  size_t i;
  for (i = 0; i < blocks.n; ++i)
    {
      if (blocks.e[i].ptr == ptr)
        {
          aggregator_bytes -= blocks.e[i].size;
          memmove(blocks.e + i, blocks.e + i + 1, (blocks.n - i - 1) * sizeof(*blocks.e));
          blocks.n--;
          return;
        }
    }

  debug_error("%p: unable to track ptr in call to free. bad ptr?", ptr);
}

void*
malloc (size_t size)
{
#ifndef HAVE___LIBC_MALLOC
  if (__libc_malloc == NULL)
    {
      guard (NULL != (__libc_malloc = dlsym(RTLD_NEXT, "malloc"))) else { return NULL; }
    }
#endif

  void *res = __libc_malloc(size);
  if (res && aggregator_enabled)
    track_alloc(size, res);
  return res;
}

void*
calloc (size_t nmemb, size_t size)
{
#ifndef HAVE___LIBC_CALLOC
  if (__libc_calloc == NULL)
    {
      guard (NULL != (__libc_calloc = dlsym(RTLD_NEXT, "calloc"))) else { return NULL; }
    }
#endif

  void *res = __libc_calloc(nmemb, size);
  if (res && aggregator_enabled)
    track_alloc(size * nmemb, res);
  return res;
}

void*
realloc (void *ptr, size_t size)
{
#ifndef HAVE___LIBC_REALLOC
  if (__libc_realloc == NULL)
    {
      guard (NULL != (__libc_realloc = dlsym(RTLD_NEXT, "realloc"))) else { return NULL; }
    }
#endif

  void *res = __libc_realloc(ptr, size);
  if (res && aggregator_enabled)
    track_realloc(ptr, size, res);
  return res;
}

void
free (void *ptr)
{
#ifndef HAVE___LIBC_FREE
  if (__libc_free == NULL)
    {
      guard (NULL != (__libc_free = dlsym(RTLD_NEXT, "free"))) else { return; }
    }
#endif

  __libc_free(ptr);
  if (aggregator_enabled)
    track_free(ptr);
}
