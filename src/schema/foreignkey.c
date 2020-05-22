
#include "schema/foreignkey.h"

#include <stdlib.h>

void
foreignkey_init (struct foreignkey_t *foreignkey)
{
  foreignkey->lhs_table = NULL;
  foreignkey->rhs_table = NULL;

  list_init(&foreignkey->_lhs);
  list_init(&foreignkey->_rhs);
}

void
foreignkey_fini (struct foreignkey_t *foreignkey)
{
  foreignkey->lhs_table = NULL;
  foreignkey->rhs_table = NULL;

  list_fini(&foreignkey->_lhs, NULL);
  free(foreignkey->lhs.columns);
  foreignkey->lhs.columns = NULL;

  list_fini(&foreignkey->_rhs, NULL);
  free(foreignkey->rhs.columns);
  foreignkey->rhs.columns = NULL;
}
