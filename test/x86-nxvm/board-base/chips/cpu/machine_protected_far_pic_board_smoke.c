#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/protected_pic_machine_fixture.h"
/* PIC delivery is board wiring; the far JMP itself is asserted by the CPU test. */
int main(void)
{
    static const lib_u8 jmp[] = {0xeau,0,0,0x18u,0,0x90u};
    static const lib_u8 target[] = {0x90u,0xf4u};
    protected_pic_board_fixture board;
    x86_core_cpu_debug_cpu_snapshot after = {0};
    x86_core_machine_run_result result = {0};
    lib_u16 frame_ip;
    lib_bool failed = !protected_pic_board_prepare(&board, LIB_FALSE);

    if (!failed) {
        failed = x86_core_machine_memory_write(board.machine, 0x4000u,
                target, sizeof(target)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(board.machine, CPU_PROTECTED_CODE_BASE,
                jmp, sizeof(jmp)) != LIB_STATUS_OK ||
            !protected_pic_board_raise(&board) ||
            x86_core_machine_run(board.machine,
                (x86_core_machine_run_budget){2u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            x86_core_machine_debug_capture_cpu_snapshot(board.machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(board.machine, CPU_PROTECTED_DATA_BASE +
                (lib_u16)after.esp, &frame_ip, sizeof(frame_ip)) != LIB_STATUS_OK ||
            after.cs.selector != 0x08u || after.eip != 0x101u || frame_ip != 0u ||
            (test_pic_read(board.board->shared_pic_master, 0x0bu) & VPIC_ISR_IRQ(0u)) == 0u ||
            (test_pic_read(board.board->shared_pic_master, 0x0au) & VPIC_IRR_IRQ(0u)) != 0u;
    }
    protected_pic_board_finalize(&board);
    if (failed) return 1;
    lib_c_printf("%s\n", "PROTECTED-FAR-PIC-BOARD:OK");
    return 0;
}
