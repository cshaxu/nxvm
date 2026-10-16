#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/protected_pic_machine_fixture.h"
/* PIC delivery after a DS read has no CPU interrupt-shadow side channel. */
int main(void)
{
    static const lib_u8 code[] = {0x8au,0x06u,0x20u,0,0x90u};
    protected_pic_board_fixture board;
    static const lib_u8 data = 0x5au;
    x86_core_cpu_debug_cpu_snapshot after = {0};
    x86_core_machine_run_result result = {0};
    lib_u16 frame_ip;
    lib_bool failed = !protected_pic_board_prepare(&board, LIB_TRUE);

    if (!failed) {
        failed = x86_core_machine_memory_write(board.machine,
                CPU_PROTECTED_DATA_BASE + 0x20u, &data, sizeof(data)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(board.machine, CPU_PROTECTED_CODE_BASE,
                code, sizeof(code)) != LIB_STATUS_OK ||
            x86_core_machine_debug_write_register(board.machine,
                X86_CORE_CPU_DEBUG_EAX, 0xaabbcc44u) != LIB_STATUS_OK ||
            !protected_pic_board_raise(&board) ||
            x86_core_machine_run(board.machine,
                (x86_core_machine_run_budget){2u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            x86_core_machine_debug_capture_cpu_snapshot(board.machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(board.machine, CPU_PROTECTED_STACK_BASE +
                (lib_u16)after.esp, &frame_ip, sizeof(frame_ip)) != LIB_STATUS_OK ||
            after.eax != 0xaabbcc5au || after.cs.selector != 0x08u ||
            after.eip != 0x101u || frame_ip != 4u || after.esp != 0x7ffau ||
            (test_pic_read(board.board->shared_pic_master, 0x0bu) & VPIC_ISR_IRQ(0u)) == 0u ||
            (test_pic_read(board.board->shared_pic_master, 0x0au) & VPIC_IRR_IRQ(0u)) != 0u;
    }
    protected_pic_board_finalize(&board);
    if (failed) return 1;
    lib_c_printf("%s\n", "PROTECTED-DATA-PIC-BOARD:OK");
    return 0;
}
