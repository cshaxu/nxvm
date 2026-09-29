#ifndef NXVM_TEST_HDC_H
#define NXVM_TEST_HDC_H
#include "app-nxvm/devices/hdc.h"

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
