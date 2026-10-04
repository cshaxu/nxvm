#include "pic_fixture.h"
#include "x86/ibmpc-common/machine_board_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86/ibmpc-common/machine_board_state.h"
#include "cpu_pic_lifecycle_fixture.h"

lib_i32 main(void)
{
    core_machine_config config = { .memory_bytes = 0u };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    failed |= core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;
    if (failed || machine == LIB_NULL) return 1;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= test_cpu_pic_binding_after_reset(machine);
    if (machine != LIB_NULL) {
        for (lib_u8 irq = 0u; irq < 16u; ++irq) {
            core_machine_pic_irq_source *source = LIB_NULL;
            if (irq == 2u) continue;
            failed |= core_machine_pic_irq_source_bind(&source,
                board->shared_pic_master, board->shared_pic_slave, irq) != LIB_STATUS_OK;
            core_machine_pic_irq_source_assert(source);
            core_machine_pic_irq_source_deassert(source);
        }
    }
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= test_cpu_pic_binding_after_reset(machine);
    failed |= machine == LIB_NULL || test_pic_read(board->shared_pic_master, 0x0au) != 0u ||
        test_pic_read(board->shared_pic_slave, 0x0au) != 0u;

    core_machine_destroy(machine);
    if (failed != 0) return 1;
    printf("M5:T295:S3:CORE-CPU-PIC-LIFECYCLE:OK\n");
    return 0;
}
