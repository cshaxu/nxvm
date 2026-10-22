#include "lib/types/test.h"
#include "lib/types/file.h"

static lib_u32 lifetime_fault;
static void unexpected_cleanup(void)
{
    lib_c_printf("unexpected cleanup\n");
    lib_c_fflush(lib_c_stdout);
}
static void fatal_exit(lib_i32 status)
{
    lib_test_assert(status == LIB_STATUS_INTERNAL_ERROR);
    lib_c_printf("fatal lifetime boundary\n");
    lib_c_fflush(lib_c_stdout);
    lib_process_exit_immediate(status);
}
#undef lib_process_exit_immediate
#define lib_process_exit_immediate fatal_exit
