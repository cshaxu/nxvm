#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "../../composition/support/protected_16_bootstrap_fixture.h"
#include "../../pic_fixture.h"
#include "x86-nxvm/board-base/machine_board_state.h"

int main(void)
{
    static const lib_u8 task_a[] = {0x2bu,0,0,6,0,0x81u,0,0};
    static const lib_u8 task_b[] = {0x2bu,0,0,7,0,0x81u,0,0};
    static const lib_u8 data_descriptor[] = {0xffu,0xffu,0,0x30u,0,0x92u,0,0};
    static const lib_u8 load_task[] = {
        0xb8u,0x10u,0,0x8eu,0xd8u,0x8eu,0xc0u,0x8eu,0xd0u,
        0xb8u,0x28u,0,0x0fu,0x00u,0xd8u
    };
    static const lib_u8 target_state[] = {
        0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,
        0,1, 2,2, 0x22u,0x22u, 0,0, 0,0, 0,0,
        0,0x80u, 0,0, 0,0, 0,0,
        0x10u,0, 8,0, 0x10u,0, 0x10u,0, 0,0
    };
    static const lib_u8 source[] = {0xb8u,0x11u,0x11u,0xeau,0,0,0x30u,0,0xf4u};
    static const lib_u8 target[] = {0xb8u,0x22u,0x22u,0xa3u,0,0,0xf4u};
    static const lib_u8 handler = 0xf4u;
    test_protected_16_machine state = {0};
    x86_nxvm_board_pic_irq_source *irq = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot after = {0};
    lib_bool failed = !test_protected_16_prepare(&state, X86_CORE_CPU_PROFILE_80386);

    if (!failed) {
        failed = !test_protected_16_write(&state, TEST_PROTECTED_16_GDT_BASE + 16u,
                data_descriptor, sizeof(data_descriptor)) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_GDT_BASE + 40u,
                task_a, sizeof(task_a)) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_GDT_BASE + 48u,
                task_b, sizeof(task_b)) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE,
                load_task, sizeof(load_task)) ||
            x86_core_machine_run(state.machine,
                (x86_core_machine_run_budget){6u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
            !test_protected_16_snapshot(&state, &after) ||
            after.tr.selector != 0x28u || after.ss.base != 0x3000u ||
            !test_protected_16_write(&state, 0x0700u, target_state, sizeof(target_state)) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE,
                source, sizeof(source)) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE + 0x100u,
                target, sizeof(target)) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE + 0x180u,
                &handler, sizeof(handler)) ||
            !test_protected_16_install_gate(&state, 0x20u, 0x180u, 8u,
                TEST_PROTECTED_INTERRUPT_GATE_16, 0u, LIB_TRUE) ||
            x86_core_machine_debug_write_register(state.machine,
                X86_CORE_CPU_DEBUG_EIP, 0u) != LIB_STATUS_OK;
    }
    if (!failed) {
        test_pic_program_vector(state.board->shared_pic_master, 0x20u);
        failed = x86_nxvm_board_pic_irq_source_bind(&irq,
            state.board->shared_pic_master, state.board->shared_pic_slave,
            0u) != LIB_STATUS_OK;
    }
    if (!failed) {
        x86_nxvm_board_pic_irq_source_assert(irq);
        failed = x86_core_machine_run(state.machine,
                (x86_core_machine_run_budget){4u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            !test_protected_16_snapshot(&state, &after) ||
            after.tr.selector != 0x30u || after.cs.selector != 8u ||
            after.eip != 0x181u ||
            (test_pic_read(state.board->shared_pic_master, 0x0bu) & 1u) == 0u ||
            (test_pic_read(state.board->shared_pic_master, 0x0au) & 1u) != 0u;
        x86_nxvm_board_pic_irq_source_deassert(irq);
    }
    test_protected_16_destroy(&state);
    if (failed) { lib_c_fprintf(lib_c_stderr, "%s", "TASK16-PIC:FAIL\n"); return 1; }
    lib_c_printf("%s\n", "TASK16-PIC:OK");
    return 0;
}
