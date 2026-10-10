#include "lib/types/file.h"
#include "cmos_fixture.h"
#include "core/board-base/machine_board_state.h"
#include "core/board-base/pic_bus_interface.h"
void test_board_cmos_advance(core_machine_board_state *board, lib_u64 ticks)
{
    x86_rtc_advance(board->shared_rtc, ticks);
    core_machine_pic_refresh(board->shared_pic_master, board->shared_pic_slave);
}

lib_u8 test_board_cmos_scan_interrupt(core_machine_board_state *board)
{
    return core_machine_pic_scan_interrupt(board->shared_pic_master,
        board->shared_pic_slave);
}

lib_u8 test_board_cmos_get_interrupt(core_machine_board_state *board)
{
    return core_machine_pic_get_interrupt(board->shared_pic_master,
        board->shared_pic_slave);
}

void test_board_cmos_reset(core_machine_board_state *board)
{
    x86_rtc_reset(board->shared_rtc);
}

lib_u8 test_board_cmos_read_register(const core_machine_board_state *board, lib_u8 index)
{
    return x86_rtc_read_register(board->shared_rtc, index);
}

void test_board_cmos_report_failure(core_machine_board_state *board, lib_i32 failed)
{
    lib_c_printf("RTC probe failed=%04x: second=%u hour=%u B=%02x\n", failed,
        x86_rtc_read_register(board->shared_rtc, X86_RTC_SECOND),
        x86_rtc_read_register(board->shared_rtc, X86_RTC_HOUR),
        x86_rtc_read_register(board->shared_rtc, X86_RTC_REG_B));
}
