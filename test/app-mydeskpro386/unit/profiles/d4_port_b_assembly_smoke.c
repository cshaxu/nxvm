#include "app-mydeskpro386/profiles/d4_platform_interface.h"
#include "../../../core/board-base/composition/port_assembly_fixture.h"
#include "lib/types/file.h"

static lib_status port_assembly_attach_d4(core_machine_board_state *board)
{
    const core_machine_d4_platform_config config = {CORE_MACHINE_PC_AT_PORT_B, 0u};
    core_machine_d4_platform *platform = LIB_NULL;
    return core_machine_d4_platform_attach(board, &config, &platform);
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    for (lib_size fail_at = 1u; fail_at <= 2u; ++fail_at)
        failed |= port_assembly_port_b_transaction(port_assembly_attach_d4, fail_at);

    if (failed) return 1;
    puts("D4-PORT-B-ROLLBACK:OK");
    return 0;
}
