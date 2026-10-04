#ifndef TEST_IBMPC_COMMON_HDC_FIXTURE_H
#define TEST_IBMPC_COMMON_HDC_FIXTURE_H
#include "x86/ibmpc-common/hdc.h"

static inline void test_hdc_port_write(core_machine *machine,
    lib_u16 port, lib_u32 value)
{
    if (core_machine_bus_write(machine, port, value) != LIB_STATUS_OK)
        exit(EXIT_FAILURE);
}

static inline lib_u32 test_hdc_port_read(core_machine *machine, lib_u16 port)
{
    lib_u32 value;
    if (core_machine_bus_read(machine, port, &value) != LIB_STATUS_OK)
        exit(EXIT_FAILURE);
    return value;
}

static inline x86_hdc_observation hdc_observe(const core_machine_hdc *hdc)
{
    x86_hdc_observation result = {0};
    if (hdc != LIB_NULL) (void)x86_hdc_capture(hdc->chip, &result);
    return result;
}

static inline void hdc_service(core_machine_hdc *hdc)
{
    lib_u64 due;
    if (core_machine_hdc_next_due_tick(hdc, &due) == LIB_STATUS_OK)
        core_machine_hdc_advance_at(hdc, due);
}
#endif
