#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86-nxvm/machine/machine_private.h"

#include "../../x86-nxvm/board-base/support/composition_fixture.h"
#include "x86-core/machine/debug_interface.h"

#include "../support/rom/session_assets.h"

static lib_bool sessions_are_isolated(x86_core_machine *first, x86_core_machine *second,
    x86_nxvm_board_state *first_board, x86_nxvm_board_state *second_board)
{
    const lib_u8 values[] = { 0x11u, 0x22u };
    x86_core_machine *machines[] = { first, second };
    lib_u8 observed;
    lib_u8 enabled;
    lib_u32 address;
    lib_u32 eax;
    lib_u32 index;

    if (first == LIB_NULL || second == LIB_NULL || first == second ||
        !test_board_instances_are_distinct(first_board, second_board)) return LIB_FALSE;
    for (index = 0u; index < 2u; ++index) {
        if (x86_core_machine_debug_write_real(machines[index], 0u, 0u,
                &values[index], 1u) != LIB_STATUS_OK ||
            x86_core_machine_debug_write_register(machines[index],
                X86_CORE_CPU_DEBUG_EAX, values[index] * 0x01010101u) !=
                    LIB_STATUS_OK) return LIB_FALSE;
    }
    if (x86_core_machine_debug_set_watchpoint(first, X86_CORE_CPU_DEBUG_WATCH_READ,
            0x1234u) != LIB_STATUS_OK) return LIB_FALSE;
    for (index = 0u; index < 2u; ++index) {
        if (x86_core_machine_debug_read_real(machines[index], 0u, 0u,
                &observed, 1u) != LIB_STATUS_OK || observed != values[index] ||
            x86_core_machine_debug_read_register(machines[index],
                X86_CORE_CPU_DEBUG_EAX, &eax) != LIB_STATUS_OK ||
            eax != values[index] * 0x01010101u ||
            x86_core_machine_debug_get_watchpoint(machines[index],
                X86_CORE_CPU_DEBUG_WATCH_READ, &enabled, &address) !=
                    LIB_STATUS_OK || enabled != (index == 0u) ||
            (enabled && address != 0x1234u)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    x86_nxvm_machine *first = LIB_NULL;
    x86_nxvm_machine *second = LIB_NULL;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &first) != LIB_STATUS_OK ||
        first == LIB_NULL ||
        vm_test_default_pc_at_session_create(LIB_NULL, &second) != LIB_STATUS_OK ||
        second == LIB_NULL) failed = 1;

    if (!failed) {
        failed = first->fdc_dma_request.core_token == second->fdc_dma_request.core_token;
        failed = failed || !sessions_are_isolated(
            first->x86_core_machine, second->x86_core_machine, first->board, second->board);
    }

    x86_nxvm_machine_destroy(second);
    x86_nxvm_machine_destroy(first);

    if (failed) return 1;
    lib_c_printf("TWO-SESSION-ISOLATION:OK\n");
    return 0;
}
