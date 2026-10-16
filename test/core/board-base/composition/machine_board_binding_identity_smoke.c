#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "../board_binding_fixture.h"

lib_i32 main(void)
{
    const core_machine_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_8086,
        .ticks_per_instruction = 1u
    };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_attachment expected;
    lib_bool failed;
    if (test_board_binding_create(&config, &machine, &expected) != LIB_STATUS_OK) return 1;
    failed = expected.context == LIB_NULL || expected.context == machine;
    x86_core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("%s\n", "BOARD-BINDING-IDENTITY:OK");
    return 0;
}
