#ifndef LIST_H
#define LIST_H

#include <stddef.h>

struct list_t
{
  size_t n;
  void **e;
};

void list_init (struct list_t*);

void list_fini (struct list_t*, void(*)(void*));

void list_fini_shallow (struct list_t*);

int list_add (struct list_t*, void*);

#endif
