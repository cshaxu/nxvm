#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

static C_INT core_machine_real_mode_tick_case(
    const C_CHAR *name,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes,
    core_machine_cpu_profile profile, type_status expected_status,
    core_machine_stop_reason expected_reason, type_unsigned_64 expected_executed,
    type_unsigned_64 expected_ticks)
{
    const core_machine_config config = {
        .cpu_profile = profile,
        .ticks_per_instruction = 2u
    };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_observation observation;
    core_machine_cpu_profile actual_profile;
    core_machine *machine = STD_NULL;
    type_status status;
    C_INT failed = 0;

    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff0u, program,
        program_bytes) != TYPE_STATUS_OK;
    if (expected_status == TYPE_STATUS_FAULT)
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(machine);
    status = core_machine_run(machine, budget, &result);
    failed |= status != expected_status || result.reason != expected_reason ||
        result.executed != expected_executed || result.ticks != expected_ticks ||
        result.elapsed_ticks != expected_ticks;
    failed |= core_machine_get_cpu_profile(machine, &actual_profile) !=
        TYPE_STATUS_OK || actual_profile != profile;
    failed |= core_machine_capture_observation(machine, &observation) !=
        TYPE_STATUS_OK || observation.elapsed_ticks != expected_ticks;
    if (failed) {
        STD_FPRINTF(STD_STDERR,
            "M5:T218:S2:REAL-MODE-TICKS:FAIL case=%s status=%d reason=%d "
            "executed=%llu ticks=%llu elapsed=%llu profile=%d halted=%u fault=%u\n", name, (C_INT)status,
            (C_INT)result.reason, (unsigned long long)result.executed,
            (unsigned long long)result.ticks,
            (unsigned long long)result.elapsed_ticks, (C_INT)actual_profile,
            observation.cpu.halted, observation.diagnostic.first_fault.exception_mask);
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    static const type_unsigned_8 mov_ax[] = { 0xb8u, 0x34u, 0x12u };
    static const type_unsigned_8 out_80[] = { 0xe6u, 0x80u };
    static const type_unsigned_8 int_20[] = { 0xcdu, 0x20u, 0x90u };
    static const type_unsigned_8 prefixed_nop[] = { 0x26u, 0x90u };
    static const type_unsigned_8 halt[] = { 0xf4u };
    static const type_unsigned_8 operand_size_prefix[] = { 0x66u, 0x90u };
    C_INT failed = 0;

    failed |= core_machine_real_mode_tick_case("mov", mov_ax, sizeof(mov_ax),
        CORE_MACHINE_CPU_PROFILE_80286, TYPE_STATUS_OK, CORE_MACHINE_STOP_BUDGET,
        1u, 2u);
    failed |= core_machine_real_mode_tick_case("out", out_80, sizeof(out_80),
        CORE_MACHINE_CPU_PROFILE_80286, TYPE_STATUS_OK, CORE_MACHINE_STOP_BUDGET,
        1u, 3u);
    failed |= core_machine_real_mode_tick_case("int", int_20, sizeof(int_20),
        CORE_MACHINE_CPU_PROFILE_80286, TYPE_STATUS_OK, CORE_MACHINE_STOP_BUDGET,
        1u, 25u);
    failed |= core_machine_real_mode_tick_case("segment-prefix", prefixed_nop,
        sizeof(prefixed_nop),
        CORE_MACHINE_CPU_PROFILE_80286, TYPE_STATUS_OK, CORE_MACHINE_STOP_BUDGET,
        1u, 3u);
    failed |= core_machine_real_mode_tick_case("hlt", halt, sizeof(halt),
        CORE_MACHINE_CPU_PROFILE_80286, TYPE_STATUS_OK,
        CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, 1u, 2u);
    failed |= core_machine_real_mode_tick_case("operand-size-prefix",
        operand_size_prefix,
        sizeof(operand_size_prefix), CORE_MACHINE_CPU_PROFILE_80286,
        TYPE_STATUS_FAULT, CORE_MACHINE_STOP_FAULT, 0u, 0u);
    if (failed) return 1;
    STD_PRINTF("M5:T218:S2:REAL-MODE-TICKS:OK\n");
    return 0;
}
