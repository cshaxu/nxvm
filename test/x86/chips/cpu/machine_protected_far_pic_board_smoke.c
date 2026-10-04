#include "support/protected_pic_board_fixture.h"
#include <stdio.h>

/* PIC delivery is board wiring; the far JMP itself is asserted by the CPU test. */
int main(void)
{
    static const lib_u8 jmp[] = {0xeau,0,0,0x18u,0,0x90u};
    static const lib_u8 target[] = {0x90u,0xf4u};
    protected_pic_board_fixture board;
    t_cpu after;
    lib_u16 frame_ip;
    lib_bool failed = !protected_pic_board_prepare(&board, LIB_FALSE);

    if (!failed) {
        lib_memory_copy(board.cpu.memory + 0x4000u, target, sizeof(target));
        lib_memory_copy(board.cpu.memory + CPU_PROTECTED_CODE_BASE, jmp, sizeof(jmp));
        protected_pic_board_raise(&board);
        core_machine_cpu_execution_refresh(&board.cpu.execution);
        core_machine_cpu_execution_refresh(&board.cpu.execution);
        after = board.cpu.cpu;
        lib_memory_copy(&frame_ip, board.cpu.memory + CPU_PROTECTED_DATA_BASE +
            (lib_u16)after.data.esp, sizeof(frame_ip));
        failed = board.cpu.execution.stop_requested || board.cpu.fault.valid ||
            after.data.cs.selector != 0x08u || after.data.eip != 0x101u ||
            frame_ip != 0u || !CORE_MACHINE_BIT_IS_SET(
                test_pic_read(board.master, 0x0bu), VPIC_ISR_IRQ(0u)) ||
            CORE_MACHINE_BIT_IS_SET(test_pic_read(board.master, 0x0au), VPIC_IRR_IRQ(0u));
    }
    protected_pic_board_finalize(&board);
    if (failed) return 1;
    puts("M5:T539:S53:PROTECTED-FAR-PIC-BOARD:OK");
    return 0;
}
