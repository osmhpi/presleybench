#ifndef COLUMN_H
#define COLUMN_H

struct column_t
{
  char *name;
};

void column_init (struct column_t*);

void column_fini (struct column_t*);

#endif
