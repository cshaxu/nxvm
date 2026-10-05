#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/protected_pic_board_fixture.h"
/* PIC delivery after a DS read has no CPU interrupt-shadow side channel. */
int main(void)
{
    static const lib_u8 code[] = {0x8au,0x06u,0x20u,0,0x90u};
    protected_pic_board_fixture board;
    t_cpu after;
    lib_u16 frame_ip;
    lib_bool failed = !protected_pic_board_prepare(&board, LIB_TRUE);

    if (!failed) {
        board.cpu.memory[CPU_PROTECTED_DATA_BASE + 0x20u] = 0x5au;
        lib_memory_copy(board.cpu.memory + CPU_PROTECTED_CODE_BASE, code, sizeof(code));
        board.cpu.cpu.data.eax = 0xaabbcc44u;
        protected_pic_board_raise(&board);
        core_machine_cpu_execution_refresh(&board.cpu.execution);
        core_machine_cpu_execution_refresh(&board.cpu.execution);
        after = board.cpu.cpu;
        lib_memory_copy(&frame_ip, board.cpu.memory + CPU_PROTECTED_STACK_BASE +
            (lib_u16)after.data.esp, sizeof(frame_ip));
        failed = board.cpu.execution.stop_requested || board.cpu.fault.valid ||
            after.data.eax != 0xaabbcc5au || after.data.cs.selector != 0x08u ||
            after.data.eip != 0x101u || frame_ip != 4u || after.data.esp != 0x7ffau ||
            !CORE_MACHINE_BIT_IS_SET(test_pic_read(board.master, 0x0bu), VPIC_ISR_IRQ(0u)) ||
            CORE_MACHINE_BIT_IS_SET(test_pic_read(board.master, 0x0au), VPIC_IRR_IRQ(0u));
    }
    protected_pic_board_finalize(&board);
    if (failed) return 1;
    lib_c_printf("%s\n", "M5:T539:S53:PROTECTED-DATA-PIC-BOARD:OK");
    return 0;
}
