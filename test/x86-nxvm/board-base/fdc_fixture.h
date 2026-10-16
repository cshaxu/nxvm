#include "lib/types/test.h"
#ifndef TEST_X86_NXVM_EMULATOR_FDC_FIXTURE_H
#define TEST_X86_NXVM_EMULATOR_FDC_FIXTURE_H
#include "x86-nxvm/board-base/fdc.h"
#include "fdc_values.h"

static inline void test_fdc_port_write(x86_core_machine *machine,
    lib_u16 port, lib_u32 value)
{
    if (x86_core_machine_bus_write(machine, port, value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
}

static inline lib_u32 test_fdc_port_read(x86_core_machine *machine, lib_u16 port)
{
    lib_u32 value;
    if (x86_core_machine_bus_read(machine, port, &value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return value;
}

static inline lib_bool test_fdc_advance_due(x86_nxvm_board_fdc *fdc)
{
    lib_u64 due;
    if (x86_nxvm_board_fdc_next_due_tick(fdc, &due) != LIB_STATUS_OK) return LIB_FALSE;
    x86_nxvm_board_fdc_advance_at(fdc, due);
    return LIB_TRUE;
}

static inline lib_bool test_fdc_advance_ticks(x86_nxvm_board_fdc *fdc, lib_u64 ticks)
{
    x86_fdc_observation observation;
    if (x86_fdc_capture(fdc->chip, &observation) != LIB_STATUS_OK ||
        ticks > LIB_UINT64_MAX - observation.elapsed_ticks) return LIB_FALSE;
    x86_nxvm_board_fdc_advance_at(fdc, observation.elapsed_ticks + ticks);
    return LIB_TRUE;
}

static inline void test_fdc_advance(x86_nxvm_board_fdc *fdc)
{
    if (!test_fdc_advance_ticks(fdc, 1u)) lib_test_assert(LIB_FALSE);
}

static inline lib_bool test_fdc_finish_seeks(x86_nxvm_board_fdc *fdc)
{
    /* Bound the fixture, not the chip: four recalibrations need at most
     * 4*77 step events. Other fixtures may use individual deadlines instead. */
    for (lib_u32 event = 0u; event < 1024u; ++event) {
        if ((x86_fdc_read_status(fdc->chip) & 0x0fu) == 0u) return LIB_TRUE;
        if (!test_fdc_advance_due(fdc)) return LIB_FALSE;
    }
    return LIB_FALSE;
}

static inline lib_bool test_fdc_finish_command_wait(x86_nxvm_board_fdc *fdc)
{
    /* One active load plus at most four retained unload deadlines. Do not
     * service data bytes: a ready RQM/DRQ ends this command-entry wait. */
    for (lib_u8 event = 0u; event < X86_FDC_DRIVE_COUNT + 2u; ++event) {
        x86_fdc_observation observation;
        lib_u64 due;
        if (x86_fdc_capture(fdc->chip, &observation) != LIB_STATUS_OK) return LIB_FALSE;
        if ((observation.msr & 0xf0u) != 0x10u || observation.drq) return LIB_TRUE;
        if (x86_nxvm_board_fdc_next_due_tick(fdc, &due) != LIB_STATUS_OK ||
            due <= observation.elapsed_ticks) return LIB_TRUE;
        if (event == X86_FDC_DRIVE_COUNT + 1u) return LIB_FALSE;
        x86_nxvm_board_fdc_advance_at(fdc, due);
    }
    return LIB_FALSE;
}

static inline lib_bool test_fdc_interrupt_matches(const x86_nxvm_board_fdc *fdc,
    lib_bool asserted)
{
    x86_fdc_observation observation;
    return x86_fdc_capture(fdc->chip, &observation) == LIB_STATUS_OK &&
        observation.interrupt_pending == asserted &&
        x86_nxvm_board_pic_irq_source_is_asserted(fdc->connect.irq_source) == asserted;
}
#endif
