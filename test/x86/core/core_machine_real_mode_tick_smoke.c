#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86/core/machine_interface.h"
#include "x86/core/debug_interface.h"
#include "memory_alias_fixture.h"

/* REAL_UD_RECEIVERLESS_LIDT: preparation is outside measured ticks. */

static lib_i32 core_machine_real_mode_tick_case(
    const char *name,
    const lib_u8 *program, lib_size program_bytes,
    core_machine_cpu_profile profile, lib_status expected_status,
    core_machine_stop_reason expected_reason, lib_u64 expected_executed,
    lib_u64 expected_ticks)
{
    const core_machine_executor_config config = {
        .cpu_profile = profile,
        .ticks_per_instruction = 2u
    };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result = {0};
    core_machine_observation observation = {0};
    core_machine_cpu_profile actual_profile = CORE_MACHINE_CPU_PROFILE_8086;
    core_machine *machine = LIB_NULL;
    lib_status status;
    lib_u64 setup_ticks = 0u;
    lib_i32 failed = 0;

    failed |= core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    if (expected_status == LIB_STATUS_INTERNAL_ERROR && !failed) {
        const lib_u8 lidt[] = {0x0fu, 0x01u, 0x1eu, 0x00u, 0x06u};
        const lib_u8 idt_pointer[] = {0x17u, 0u, 0u, 0u, 0u, 0u};
        core_machine_run_result setup;

        failed |= core_machine_memory_write(machine, 0xfffffff0u, lidt,
            sizeof(lidt)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0600u, idt_pointer,
            sizeof(idt_pointer)) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u, 0u},
            &setup) != LIB_STATUS_OK || setup.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_debug_write_register(machine, CORE_MACHINE_DEBUG_EIP,
            0xfff0u) != LIB_STATUS_OK ||
            core_machine_capture_observation(machine, &observation) != LIB_STATUS_OK;
        if (failed) {
            core_machine_destroy(machine);
            return 1;
        }
        setup_ticks = observation.elapsed_ticks;
    }
    failed |= core_machine_memory_write(machine, 0xfffffff0u, program,
        program_bytes) != LIB_STATUS_OK;
    status = core_machine_run(machine, budget, &result);
    failed |= status != expected_status || result.reason != expected_reason ||
        result.executed != expected_executed || result.ticks != expected_ticks ||
        result.elapsed_ticks != setup_ticks + expected_ticks;
    failed |= core_machine_get_cpu_profile(machine, &actual_profile) !=
        LIB_STATUS_OK || actual_profile != profile;
    failed |= core_machine_capture_observation(machine, &observation) !=
        LIB_STATUS_OK || observation.elapsed_ticks != setup_ticks + expected_ticks;
    if (failed) {
        lib_c_fprintf(lib_c_stderr,
            "REAL-MODE-TICKS:FAIL case=%s status=%d reason=%d "
            "executed=%llu ticks=%llu elapsed=%llu profile=%d halted=%u fault=%u\n", name, (lib_i32)status,
            (lib_i32)result.reason, (unsigned long long)result.executed,
            (unsigned long long)result.ticks,
            (unsigned long long)result.elapsed_ticks, (lib_i32)actual_profile,
            observation.cpu.halted, observation.diagnostic.first_fault.exception_mask);
    }
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    static const lib_u8 mov_ax[] = { 0xb8u, 0x34u, 0x12u };
    static const lib_u8 out_80[] = { 0xe6u, 0x80u };
    static const lib_u8 int_20[] = { 0xcdu, 0x20u, 0x90u };
    static const lib_u8 prefixed_nop[] = { 0x26u, 0x90u };
    static const lib_u8 halt[] = { 0xf4u };
    static const lib_u8 operand_size_prefix[] = { 0x66u, 0x90u };
    lib_i32 failed = 0;

    failed |= core_machine_real_mode_tick_case("mov", mov_ax, sizeof(mov_ax),
        CORE_MACHINE_CPU_PROFILE_80286, LIB_STATUS_OK, CORE_MACHINE_STOP_BUDGET,
        1u, 2u);
    failed |= core_machine_real_mode_tick_case("out", out_80, sizeof(out_80),
        CORE_MACHINE_CPU_PROFILE_80286, LIB_STATUS_OK, CORE_MACHINE_STOP_BUDGET,
        1u, 3u);
    failed |= core_machine_real_mode_tick_case("int", int_20, sizeof(int_20),
        CORE_MACHINE_CPU_PROFILE_80286, LIB_STATUS_OK, CORE_MACHINE_STOP_BUDGET,
        1u, 25u);
    failed |= core_machine_real_mode_tick_case("segment-prefix", prefixed_nop,
        sizeof(prefixed_nop),
        CORE_MACHINE_CPU_PROFILE_80286, LIB_STATUS_OK, CORE_MACHINE_STOP_BUDGET,
        1u, 3u);
    failed |= core_machine_real_mode_tick_case("hlt", halt, sizeof(halt),
        CORE_MACHINE_CPU_PROFILE_80286, LIB_STATUS_OK,
        CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, 1u, 2u);
    failed |= core_machine_real_mode_tick_case("operand-size-prefix",
        operand_size_prefix,
        sizeof(operand_size_prefix), CORE_MACHINE_CPU_PROFILE_80286,
        LIB_STATUS_OK, CORE_MACHINE_STOP_BUDGET, 0u, 0u);
    if (failed) return 1;
    lib_c_printf("REAL-MODE-TICKS:OK\n");
    return 0;
}
