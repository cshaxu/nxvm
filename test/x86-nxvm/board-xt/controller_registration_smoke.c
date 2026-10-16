#include "controller_fixture.h"
#include "lib/types/file.h"

lib_i32 main(void)
{
    if (xt_construction_rollback()) return 1;
    lib_c_printf("XT-PPI-REGISTRATION:OK\n");
    return 0;
}
