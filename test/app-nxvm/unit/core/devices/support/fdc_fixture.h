#ifndef TEST_NXVM_FDC_FIXTURE_H
#define TEST_NXVM_FDC_FIXTURE_H
#include "app-nxvm/devices/fdc.h"

/* Wire values asserted by board tests, independent of chip-private layout. */
enum {
    TEST_FDC_MSR_CB = 0x10u,
    TEST_FDC_MSR_NDM = 0x20u,
    TEST_FDC_MSR_DIO = 0x40u,
    TEST_FDC_MSR_RQM = 0x80u,
    TEST_FDC_MSR_READY_READ = 0xc0u,
    TEST_FDC_MSR_PROCESS_READ = 0xd0u,
    TEST_FDC_MSR_RESULT = 0xd0u,
    TEST_FDC_ST0_NORMAL = 0x00u,
    TEST_FDC_ST0_SEEK_END = 0x20u,
    TEST_FDC_ST0_EQUIPMENT_CHECK = 0x10u,
    TEST_FDC_ST0_ABNORMAL = 0x40u,
    TEST_FDC_ST0_READY_CHANGE = 0xc0u,
    TEST_FDC_ST2_SCAN_MATCH = 0x08u,
    TEST_FDC_ST2_SCAN_MISMATCH = 0x04u,
    TEST_FDC_ST2_CONTROL_MARK = 0x40u
};

static inline lib_bool test_fdc_advance_due(core_machine_fdc *fdc)
{
    lib_u64 due;
    if (core_machine_fdc_next_due_tick(fdc, &due) != LIB_STATUS_OK) return LIB_FALSE;
    core_machine_fdc_advance_at(fdc, due);
    return LIB_TRUE;
}

static inline lib_bool test_fdc_advance_ticks(core_machine_fdc *fdc, lib_u64 ticks)
{
    x86_fdc_observation observation;
    if (x86_fdc_capture(fdc->chip, &observation) != LIB_STATUS_OK ||
        ticks > LIB_UINT64_MAX - observation.elapsed_ticks) return LIB_FALSE;
    core_machine_fdc_advance_at(fdc, observation.elapsed_ticks + ticks);
    return LIB_TRUE;
}

static inline void test_fdc_advance(core_machine_fdc *fdc)
{
    if (!test_fdc_advance_ticks(fdc, 1u)) exit(EXIT_FAILURE);
}

static inline lib_bool test_fdc_finish_seeks(core_machine_fdc *fdc)
{
    /* Bound the fixture, not the chip: four recalibrations need at most
     * 4*77 step events. Other fixtures may use individual deadlines instead. */
    for (lib_u32 event = 0u; event < 1024u; ++event) {
        if ((x86_fdc_read_status(fdc->chip) & 0x0fu) == 0u) return LIB_TRUE;
        if (!test_fdc_advance_due(fdc)) return LIB_FALSE;
    }
    return LIB_FALSE;
}

static inline lib_bool test_fdc_interrupt_matches(const core_machine_fdc *fdc,
    lib_bool asserted)
{
    x86_fdc_observation observation;
    return x86_fdc_capture(fdc->chip, &observation) == LIB_STATUS_OK &&
        observation.interrupt_pending == asserted &&
        core_machine_pic_irq_source_is_asserted(fdc->connect.irq_source) == asserted;
}
#endif
