#ifndef REFERENCE_H
#define REFERENCE_H

enum reference_e
{
  REFERENCE_NONE,
  REFERENCE_TABLE,
  REFERENCE_COLUMN,
};

struct reference_t
{
  enum reference_e type;
  void *ref;
};

#endif
