#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "../cpu_pic_lifecycle_fixture.h"

lib_i32 main(void)
{
    x86_nxvm_board_config config = { .memory_bytes = 0u };
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    failed |= x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK;
    if (failed || machine == LIB_NULL) return 1;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= test_cpu_pic_binding_after_reset(machine);
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= test_cpu_pic_binding_after_reset(machine);

    x86_core_machine_destroy(machine);
    if (failed != 0) return 1;
    lib_c_printf("CORE-CPU-RESET-IDENTITY:OK\n");
    return 0;
}
