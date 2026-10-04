#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/machine/machine_private.h"

#include "../../../../x86/core/composition_fixture.h"
#include "../../../../x86/ibmpc-common/composition_fixture.h"
#include "x86/core/debug_interface.h"

#include "app-nxvm/machine/machine_interface.h"
#include "support/rom/session_assets.h"

static lib_bool sessions_are_isolated(core_machine *first, core_machine *second,
    core_machine_board_state *first_board, core_machine_board_state *second_board)
{
    const lib_u8 values[] = { 0x11u, 0x22u };
    core_machine *machines[] = { first, second };
    lib_u8 observed;
    lib_u8 enabled;
    lib_u32 address;
    lib_u32 eax;
    lib_u32 index;

    if (!test_core_instances_are_distinct(first, second) ||
        !test_board_instances_are_distinct(first_board, second_board)) return LIB_FALSE;
    for (index = 0u; index < 2u; ++index) {
        if (core_machine_debug_write_real(machines[index], 0u, 0u,
                &values[index], 1u) != LIB_STATUS_OK ||
            core_machine_debug_write_register(machines[index],
                CORE_MACHINE_DEBUG_EAX, values[index] * 0x01010101u) !=
                    LIB_STATUS_OK) return LIB_FALSE;
    }
    if (core_machine_debug_set_watchpoint(first, CORE_MACHINE_DEBUG_WATCH_READ,
            0x1234u) != LIB_STATUS_OK) return LIB_FALSE;
    for (index = 0u; index < 2u; ++index) {
        if (core_machine_debug_read_real(machines[index], 0u, 0u,
                &observed, 1u) != LIB_STATUS_OK || observed != values[index] ||
            core_machine_debug_read_register(machines[index],
                CORE_MACHINE_DEBUG_EAX, &eax) != LIB_STATUS_OK ||
            eax != values[index] * 0x01010101u ||
            core_machine_debug_get_watchpoint(machines[index],
                CORE_MACHINE_DEBUG_WATCH_READ, &enabled, &address) !=
                    LIB_STATUS_OK || enabled != (index == 0u) ||
            (enabled && address != 0x1234u)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    vm_machine *first = LIB_NULL;
    vm_machine *second = LIB_NULL;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &first) != LIB_STATUS_OK ||
        first == LIB_NULL ||
        vm_test_default_pc_at_session_create(LIB_NULL, &second) != LIB_STATUS_OK ||
        second == LIB_NULL) failed = 1;

    if (!failed) {
        failed = first->fdc_dma_request.core_token == second->fdc_dma_request.core_token;
        failed = failed || !sessions_are_isolated(
            first->core_machine, second->core_machine, first->board, second->board);
    }

    vm_machine_destroy(second);
    vm_machine_destroy(first);

    if (failed) return 1;
    puts("M5:T73:S1:TWO-SESSION-ISOLATION:OK");
    return 0;
}
