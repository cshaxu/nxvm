#include "lib/types/test.h"
#ifndef TEST_IBMPC_EMULATOR_HDC_FIXTURE_H
#define TEST_IBMPC_EMULATOR_HDC_FIXTURE_H
#include "x86-nxvm/board-base/hdc.h"

static inline void test_hdc_port_write(x86_core_machine *machine,
    lib_u16 port, lib_u32 value)
{
    if (x86_core_machine_bus_write(machine, port, value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
}

static inline lib_u32 test_hdc_port_read(x86_core_machine *machine, lib_u16 port)
{
    lib_u32 value;
    if (x86_core_machine_bus_read(machine, port, &value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return value;
}

static inline x86_hdc_observation hdc_observe(const x86_nxvm_board_hdc *hdc)
{
    x86_hdc_observation result = {0};
    if (hdc != LIB_NULL)
        (void)x86_hdc_capture(hdc->chips[hdc->selected_controller], &result);
    return result;
}

static inline void hdc_service(x86_nxvm_board_hdc *hdc)
{
    lib_u64 due;
    if (x86_nxvm_board_hdc_next_due_tick(hdc, &due) == LIB_STATUS_OK)
        x86_nxvm_board_hdc_advance_at(hdc, due);
}
#endif
