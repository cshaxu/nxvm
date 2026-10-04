#ifndef TEST_X86_CORE_DEBUG_FIXTURE_H
#define TEST_X86_CORE_DEBUG_FIXTURE_H

#include "lib/types/types_interface.h"
#include "x86/core/debug_interface.h"

static inline lib_u32 test_core_machine_fixture_read_register(
    const core_machine *machine, core_machine_debug_register register_id)
{
    lib_u32 value;
    if (core_machine_debug_read_register(machine, register_id, &value) != LIB_STATUS_OK)
        exit(EXIT_FAILURE);
    return value;
}

static inline void test_core_machine_fixture_write_register(
    core_machine *machine, core_machine_debug_register register_id, lib_u32 value)
{
    if (core_machine_debug_write_register(machine, register_id, value) != LIB_STATUS_OK)
        exit(EXIT_FAILURE);
}

/* Historical word seeds change the low half, not the whole 32-bit register. */
static inline void test_core_machine_fixture_write_word(
    core_machine *machine, core_machine_debug_register register_id, lib_u16 value)
{
    const lib_u32 previous = test_core_machine_fixture_read_register(machine, register_id);
    test_core_machine_fixture_write_register(machine, register_id,
        (previous & 0xffff0000u) | value);
}

/* Preserve the production delivery/handler rounds for corpus tests whose
 * assertions observe the executed exception handler. */
static inline lib_status test_core_machine_fixture_run_after_delivery(
    core_machine *machine, core_machine_run_budget budget,
    core_machine_run_result *out_result)
{
    lib_status status = core_machine_run(machine, budget, out_result);
    core_machine_cpu_diagnostic diagnostic;
    lib_u8 delivered = LIB_FALSE;

    if (status == LIB_STATUS_OK && out_result != LIB_NULL &&
        out_result->reason == CORE_MACHINE_STOP_BUDGET &&
        core_machine_get_cpu_diagnostic(machine, &diagnostic) == LIB_STATUS_OK) {
        delivered = diagnostic.last_delivered_exception.valid;
    }
    if (delivered) {
        status = core_machine_run(machine, budget, out_result);
    }
    return status;
}

#endif
