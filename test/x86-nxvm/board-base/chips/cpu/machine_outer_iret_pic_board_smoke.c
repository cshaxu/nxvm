#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "../../composition/support/protected_16_bootstrap_fixture.h"
#include "../../pic_fixture.h"
#include "x86-nxvm/board-base/machine_board_state.h"

int main(void)
{
    static const lib_u8 gdt_pointer[] = {0x4fu,0,0,3,0,0};
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0xfau,0,0,
        0xffu,0xffu,0,0x50u,0,0xf2u,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0x67u,0,0,6,0,0x89u,0,0
    };
    /* LGDT uses the still-zero DS base; far JMP loads the 32-bit CS. */
    static const lib_u8 load_segments[] = {
        0x0fu,0x01u,0x16u,0,1,0xb8u,0x10u,0,0x8eu,0xd0u,
        0xeau,0x80u,0,8,0
    };
    static const lib_u8 load_task[] = {0x66u,0xb8u,0x48u,0,0x0fu,0,0xd8u};
    static const lib_u8 tss[] = {0,0,0,0,0,0x90u,0,0,0x10u,0};
    static const lib_u8 iret = 0xcfu, nop = 0x90u, hlt = 0xf4u;
    static const lib_u32 outer_frame[] = {0x10u,0x1bu,0x202u,0x1000u,0x23u};
    test_protected_16_machine state = {0};
    x86_nxvm_board_pic_irq_source *irq = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot after = {0};
    lib_bool failed = !test_protected_16_prepare(&state, X86_CORE_CPU_PROFILE_80386);

    if (!failed) {
        failed = !test_protected_16_write(&state, TEST_PROTECTED_16_GDT_POINTER,
                gdt_pointer, sizeof(gdt_pointer)) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_GDT_BASE, gdt, sizeof(gdt)) ||
            !test_protected_16_write(&state, 0x0600u, tss, sizeof(tss)) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE,
                load_segments, sizeof(load_segments)) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE + 0x80u,
                load_task, sizeof(load_task)) ||
            x86_core_machine_run(state.machine,
                (x86_core_machine_run_budget){6u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
            !test_protected_16_snapshot(&state, &after) ||
            after.tr.selector != 0x48u || after.ss.base != 0x3000u ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE, &iret, sizeof(iret)) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE + 0x100u,
                &hlt, sizeof(hlt)) ||
            !test_protected_16_write(&state, 0x4010u, &nop, sizeof(nop)) ||
            !test_protected_16_write(&state, 0xb000u, outer_frame, sizeof(outer_frame)) ||
            !test_protected_16_install_gate(&state, 0x20u, 0x100u, 8u,
                TEST_PROTECTED_INTERRUPT_GATE_16, 0u, LIB_TRUE) ||
            x86_core_machine_debug_write_register(state.machine,
                X86_CORE_CPU_DEBUG_EIP, 0u) != LIB_STATUS_OK ||
            x86_core_machine_debug_write_register(state.machine,
                X86_CORE_CPU_DEBUG_ESP, 0x12348000u) != LIB_STATUS_OK;
    }
    if (!failed) {
        test_pic_program_vector(state.board->shared_pic_master, 0x20u);
        failed = x86_nxvm_board_pic_irq_source_bind(&irq,
            state.board->shared_pic_master, state.board->shared_pic_slave,
            0u) != LIB_STATUS_OK;
    }
    if (!failed) {
        x86_nxvm_board_pic_irq_source_assert(irq);
        x86_nxvm_board_pic_irq_source_deassert(irq);
        failed = x86_core_machine_run(state.machine,
                (x86_core_machine_run_budget){4u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            !test_protected_16_snapshot(&state, &after) ||
            after.cs.selector != 8u || after.ss.selector != 0x10u ||
            after.eip != 0x101u ||
            (test_pic_read(state.board->shared_pic_master, 0x0bu) & 1u) == 0u ||
            (test_pic_read(state.board->shared_pic_master, 0x0au) & 1u) != 0u;
    }
    test_protected_16_destroy(&state);
    if (failed) {
        lib_c_fprintf(lib_c_stderr, "outer PIC cs=%04x ss=%04x ip=%08x sp=%08x flags=%08x\n",
            after.cs.selector, after.ss.selector, after.eip, after.esp, after.eflags);
        return 1;
    }
    lib_c_printf("%s\n", "OUTER-IRET:PIC-BOARD:OK");
    return 0;
}
