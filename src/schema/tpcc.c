
#include "tpcc.h"

const char *default_tpcc = "\n"
                           "table {\n"
                           "  name=\"warehouse\"\n"
                           "\n"
                           "  column { name=\"w_id\"; type=smallint; primary key }\n"
                           "  column { name=\"w_name\"; type=char(10) }\n"
                           "  column { name=\"w_street_1\"; type=char(20) }\n"
                           "  column { name=\"w_street_2\"; type=char(20) }\n"
                           "  column { name=\"w_city\"; type=char(20) }\n"
                           "  column { name=\"w_state\"; type=char(2) }\n"
                           "  column { name=\"w_zip\"; type=char(9) }\n"
                           "  column { name=\"w_tax\"; type=real }\n"
                           "  column { name=\"w_ytd\"; type=float }\n"
                           "}\n";
