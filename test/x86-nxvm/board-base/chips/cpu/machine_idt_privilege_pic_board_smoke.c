#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "../../composition/support/protected_16_bootstrap_fixture.h"
#include "../../pic_fixture.h"
#include "x86-nxvm/board-base/machine_board_state.h"

/* Enter CPL 3 with guest LTR/IRET, then deliver the board's real PIC IRQ.
 * All CPU observations are copied through the public Machine contract. */
int main(void)
{
    static const lib_u8 user_descriptor[] = {
        0xffu,0xffu,0,0x30u,0,0xfau,0x40u,0
    };
    static const lib_u8 user_stack_descriptor[] = {
        0xffu,0xffu,0,0,0,0xf2u,0xcfu,0
    };
    static const lib_u8 user_code = 0x90u;
    static const lib_u8 handler = 0xf4u;
    static const lib_u32 esp0 = 0x9000u;
    test_protected_16_machine state = {0};
    x86_nxvm_board_pic_irq_source *irq = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot after = {0};
    lib_bool failed = !test_protected_32_prepare(&state);

    if (!failed) {
        failed = !test_protected_16_write(&state, TEST_PROTECTED_16_GDT_BASE + 24u,
                user_descriptor, sizeof(user_descriptor)) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_GDT_BASE + 32u,
                user_stack_descriptor, sizeof(user_stack_descriptor)) ||
            !test_protected_32_enter_user(&state) ||
            !test_protected_16_write(&state, 0x0504u, &esp0, sizeof(esp0)) ||
            !test_protected_16_write(&state, 0x3000u, &user_code, sizeof(user_code)) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE + 0x100u,
                &handler, sizeof(handler)) ||
            !test_protected_16_install_gate(&state, 0x30u, 0x100u, 8u,
                TEST_PROTECTED_INTERRUPT_GATE_32, 0u, LIB_TRUE) ||
            x86_core_machine_debug_write_register(state.machine,
                X86_CORE_CPU_DEBUG_EIP, 0u) != LIB_STATUS_OK ||
            x86_core_machine_debug_write_register(state.machine,
                X86_CORE_CPU_DEBUG_ESP, 0x8800u) != LIB_STATUS_OK;
    }
    if (!failed) {
        test_pic_program_vector(state.board->shared_pic_master, 0x30u);
        failed = x86_nxvm_board_pic_irq_source_bind(&irq,
            state.board->shared_pic_master, state.board->shared_pic_slave,
            0u) != LIB_STATUS_OK;
    }
    if (!failed) {
        x86_nxvm_board_pic_irq_source_assert(irq);
        x86_nxvm_board_pic_irq_source_deassert(irq);
        failed = x86_core_machine_run(state.machine,
                (x86_core_machine_run_budget){2u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            !test_protected_16_snapshot(&state, &after) ||
            after.cs.selector != 0x0008u || after.cs.dpl != 0u ||
            after.ss.selector != 0x0010u || after.esp != 0x00008fecu ||
            (after.eflags & X86_CORE_CPU_DEBUG_EFLAGS_IF) != 0u ||
            (test_pic_read(state.board->shared_pic_master, 0x0bu) & 1u) == 0u ||
            (test_pic_read(state.board->shared_pic_master, 0x0au) & 1u) != 0u;
    }
    test_protected_16_destroy(&state);
    if (failed) return 1;
    lib_c_printf("%s\n", "M5::IDT-PRIVILEGE-ENTRY:PIC:OK");
    return 0;
}
