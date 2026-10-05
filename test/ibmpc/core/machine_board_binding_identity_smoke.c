#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86/core/machine.h"
#include "../board-common/board_binding_fixture.h"

lib_i32 main(void)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .ticks_per_instruction = 1u
    };
    core_machine *machine = LIB_NULL;
    core_machine_attachment expected;
    lib_bool failed;
    if (test_board_binding_create(&config, &machine, &expected) != LIB_STATUS_OK) return 1;
    failed = machine->attachment.context != expected.context ||
        machine->attachment.context == machine ||
        machine->attachment.reset_devices !=
            expected.reset_devices ||
        machine->attachment.reset_clocks != expected.reset_clocks ||
        machine->attachment.refresh_nmi != expected.refresh_nmi ||
        machine->attachment.finalize_devices != expected.finalize_devices;
    core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("%s\n", "M5:T540:S93:BOARD-BINDING-IDENTITY:OK");
    return 0;
}
