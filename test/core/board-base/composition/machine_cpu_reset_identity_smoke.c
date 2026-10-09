#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "core/x86/machine.h"
#include "../cpu_pic_lifecycle_fixture.h"

lib_i32 main(void)
{
    core_machine_config config = { .memory_bytes = 0u };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_cpu_execution_context *cpu;
    lib_i32 failed = 0;

    failed |= core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;
    if (failed || machine == LIB_NULL) return 1;
    cpu = machine->executor_cpu_execution;
    failed |= cpu == LIB_NULL;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= cpu == LIB_NULL || machine->executor_cpu_execution != cpu;
    failed |= test_cpu_pic_binding_after_reset(machine);
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= cpu == LIB_NULL || machine->executor_cpu_execution != cpu;
    failed |= test_cpu_pic_binding_after_reset(machine);

    core_machine_destroy(machine);
    if (failed != 0) return 1;
    lib_c_printf("CORE-CPU-RESET-IDENTITY:OK\n");
    return 0;
}
