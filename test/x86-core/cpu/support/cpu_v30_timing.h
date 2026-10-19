#ifndef TEST_CPU_V30_TIMING_H
#define TEST_CPU_V30_TIMING_H

#include "x86-core/cpu/cpu_instructions.h"
#include "lib/types/file.h"

/* Callers supply independent source/L2 nominal rows, keys and outcome inputs.
 * The separately provider-verified fetch observation transports that oracle
 * into the full runtime contract; no production timing evaluator is called. */
static inline lib_i32 v30_timing_check(const x86_core_cpu_execution_context *execution,
    x86_core_cpu_timing_result expected, lib_u32 fetch)
{
    const x86_core_cpu_timing_result actual=x86_core_cpu_capture_timing(execution);
    if (expected.ticks) {
        expected.ticks+=fetch;
        if (fetch) {
            expected.retirement_origin=X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_ELAPSED_L2;
            expected.formula_inputs|=X86_CORE_CPU_TIMING_INPUT_FETCH;
        }
    }
    if (actual.ticks!=expected.ticks || actual.event_ticks!=expected.event_ticks ||
        actual.key_id!=expected.key_id || actual.form_id!=expected.form_id ||
        actual.formula_inputs!=expected.formula_inputs || actual.repeat_phase!=expected.repeat_phase ||
        actual.retirement_origin!=expected.retirement_origin || actual.source_timing_unallocated ||
        x86_core_cpu_timing_maximum_ticks(X86_CORE_CPU_PROFILE_V30,LIB_NULL)) {
        lib_c_printf("V30:TIMING:FAIL ticks=%llu/%llu event=%llu/%llu key=%x/%x inputs=%x/%x origin=%u/%u\n",
            actual.ticks,expected.ticks,actual.event_ticks,expected.event_ticks,
            actual.key_id,expected.key_id,actual.formula_inputs,expected.formula_inputs,
            (lib_u32)actual.retirement_origin,(lib_u32)expected.retirement_origin);
        return 1;
    }
    return 0;
}

#endif
