#ifndef TEST_CORE_MACHINE_TIME_FIXTURE_H
#define TEST_CORE_MACHINE_TIME_FIXTURE_H
#include "x86/core/machine_interface.h"

/* Exact synthetic intervals for owner tests, not a production time API. */
lib_status test_core_machine_advance_time(core_machine *machine, lib_u64 ticks);
lib_status test_core_schedule_counter(core_machine *machine, lib_u64 due_tick,
    lib_u32 *count);
#endif
