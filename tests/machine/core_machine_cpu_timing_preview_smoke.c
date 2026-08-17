#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/cpu_instructions.h"
#include "../support/core_machine_cpu_fixture.h"

#define PREVIEW_RESET_PHYSICAL 0x000ffff0u

static C_INT preview_expect(const type_unsigned_8 *bytes,
    type_unsigned_8 available_bytes, core_machine_cpu_profile profile,
    type_bool code_32, type_unsigned_8 expected_bytes,
    type_unsigned_8 expected_components)
{
    core_machine_cpu_instruction_lexeme lexeme;

    return core_machine_cpu_instruction_lexeme_scan(bytes, available_bytes,
        profile, code_32, &lexeme) && lexeme.available &&
        lexeme.byte_count == expected_bytes &&
        lexeme.component_count == expected_components;
}

static C_INT preview_test_layouts(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 mov_imm32[] = {
        0x66u, 0xb8u, 0x78u, 0x56u, 0x34u, 0x12u
    };
    static const type_unsigned_8 mov_sib_disp[] = {
        0x67u, 0x8bu, 0x84u, 0x88u, 0x78u, 0x56u, 0x34u, 0x12u
    };
    static const type_unsigned_8 near_jcc[] = {
        0x0fu, 0x84u, 0x78u, 0x56u, 0x34u, 0x12u
    };
    static const type_unsigned_8 clts[] = { 0x0fu, 0x06u };
    static const type_unsigned_8 shld_imm8[] = {
        0x0fu, 0xa4u, 0xc1u, 0x05u
    };
    static const type_unsigned_8 rep_movsb[] = { 0xf3u, 0xa4u };
    static const type_unsigned_8 repeated_operand_prefix[] = {
        0x66u, 0x66u, 0xb8u, 0x78u, 0x56u, 0x34u, 0x12u
    };
    static const type_unsigned_8 repeated_address_prefix[] = {
        0x67u, 0x67u, 0x8au, 0x06u
    };

    return preview_expect(nop, sizeof(nop), CORE_MACHINE_CPU_PROFILE_8086,
        TYPE_FALSE, 1u, 1u) &&
        preview_expect(mov_imm32, sizeof(mov_imm32),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 6u, 3u) &&
        preview_expect(mov_sib_disp, sizeof(mov_sib_disp),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 8u, 5u) &&
        preview_expect(near_jcc, sizeof(near_jcc),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 6u, 3u) &&
        preview_expect(clts, sizeof(clts), CORE_MACHINE_CPU_PROFILE_80386,
            TYPE_TRUE, 2u, 2u) &&
        preview_expect(shld_imm8, sizeof(shld_imm8),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 4u, 4u) &&
        preview_expect(rep_movsb, sizeof(rep_movsb),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 2u, 2u) &&
        preview_expect(repeated_operand_prefix, sizeof(repeated_operand_prefix),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 7u, 4u) &&
        preview_expect(repeated_address_prefix, sizeof(repeated_address_prefix),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u);
}

static C_INT preview_test_unavailable(C_VOID)
{
    static const type_unsigned_8 truncated[] = { 0x8bu };
    static const type_unsigned_8 legacy_operand_prefix[] = { 0x66u, 0x90u };
    static const type_unsigned_8 invalid_opcode[] = { 0x82u, 0xc0u };
    static const type_unsigned_8 locked_nop[] = { 0xf0u, 0x90u };
    core_machine_cpu_instruction_lexeme lexeme;

    return !core_machine_cpu_instruction_lexeme_scan(truncated,
            sizeof(truncated), CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE,
            &lexeme) && !lexeme.available &&
        !core_machine_cpu_instruction_lexeme_scan(legacy_operand_prefix,
            sizeof(legacy_operand_prefix), CORE_MACHINE_CPU_PROFILE_80286,
            TYPE_FALSE, &lexeme) && !lexeme.available &&
        !core_machine_cpu_instruction_lexeme_scan(invalid_opcode,
            sizeof(invalid_opcode), CORE_MACHINE_CPU_PROFILE_80386,
            TYPE_TRUE, &lexeme) && !lexeme.available &&
        !core_machine_cpu_instruction_lexeme_scan(locked_nop,
            sizeof(locked_nop), CORE_MACHINE_CPU_PROFILE_80386,
            TYPE_TRUE, &lexeme) && !lexeme.available;
}

static C_INT preview_test_cpu_fetch_nonpublication(C_VOID)
{
    static const type_unsigned_8 program[] = { 0x0fu, 0x84u, 0x78u, 0x56u };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    core_machine_cpu_instruction_lexeme lexeme;
    core_machine_observation before = { 0 };
    core_machine_observation after = { 0 };
    core_machine *machine = STD_NULL;
    type_unsigned_64 committed = 0u;
    type_unsigned_64 cancelled = 0u;
    STD_SIZE_T trace_count = 0u;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, PREVIEW_RESET_PHYSICAL, program,
            sizeof(program)) != TYPE_STATUS_OK ||
        core_machine_capture_observation(machine, &before) != TYPE_STATUS_OK;

    if (!failed) {
        committed = machine->transaction.committed_count;
        cancelled = machine->transaction.cancelled_count;
        trace_count = machine->trace.count;
        failed |= !core_machine_cpu_execution_preview_lexeme(
            &machine->executor_cpu_execution, &lexeme) || !lexeme.available ||
            lexeme.byte_count != sizeof(program) || lexeme.component_count != 3u ||
            core_machine_capture_observation(machine, &after) != TYPE_STATUS_OK ||
            STD_MEMCMP(&before, &after, sizeof(before)) != 0 ||
            machine->transaction.committed_count != committed ||
            machine->transaction.cancelled_count != cancelled ||
            machine->trace.count != trace_count;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT preview_test_limited_fetch_nonpublication(C_VOID)
{
    static const type_unsigned_8 program[] = { 0x0fu, 0x84u, 0x78u, 0x56u };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    core_machine_cpu_instruction_lexeme lexeme;
    core_machine_observation before = { 0 };
    core_machine_observation after = { 0 };
    core_machine *machine = STD_NULL;
    type_unsigned_64 committed = 0u;
    type_unsigned_64 cancelled = 0u;
    STD_SIZE_T trace_count = 0u;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_prepare_real_mode_execution(machine, 0u) ||
        ((machine->executor_cpu.data.cs.base = 0x00fffffcu),
            core_machine_memory_write(machine, 0x00fffffcu, program,
                sizeof(program)) != TYPE_STATUS_OK) ||
        core_machine_capture_observation(machine, &before) != TYPE_STATUS_OK;

    if (!failed) {
        committed = machine->transaction.committed_count;
        cancelled = machine->transaction.cancelled_count;
        trace_count = machine->trace.count;
        failed |= core_machine_cpu_execution_preview_lexeme(
            &machine->executor_cpu_execution, &lexeme) || lexeme.available ||
            core_machine_capture_observation(machine, &after) != TYPE_STATUS_OK ||
            STD_MEMCMP(&before, &after, sizeof(before)) != 0 ||
            machine->transaction.committed_count != committed ||
            machine->transaction.cancelled_count != cancelled ||
            machine->trace.count != trace_count;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT preview_test_taken_jcc_target(C_VOID)
{
    static const type_unsigned_8 program[] = { 0x75u, 0xfeu };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_cpu_instruction_lexeme lexeme;
    core_machine_observation before = { 0 };
    core_machine_observation after = { 0 };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, PREVIEW_RESET_PHYSICAL, program,
            sizeof(program)) != TYPE_STATUS_OK;

    if (!failed) {
        machine->executor_cpu.data.eflags &= ~VCPU_EFLAGS_ZF;
        if (core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            machine->executor_cpu.data.eip != 0xfff0u) {
            failed = 1;
        } else if (core_machine_capture_observation(machine, &before) !=
            TYPE_STATUS_OK || !core_machine_cpu_execution_preview_lexeme(
                &machine->executor_cpu_execution, &lexeme) || !lexeme.available ||
            lexeme.byte_count != 2u || lexeme.component_count != 2u ||
            core_machine_capture_observation(machine, &after) != TYPE_STATUS_OK ||
            STD_MEMCMP(&before, &after, sizeof(before)) != 0) {
            failed = 1;
        }
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT preview_test_taken_near_jcc_target(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0x66u, 0x0fu, 0x84u, 0x02u, 0x00u, 0x00u, 0x00u,
        0x90u, 0x90u, 0xf4u
    };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_cpu_instruction_lexeme lexeme;
    core_machine_observation before = { 0 };
    core_machine_observation after = { 0 };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, PREVIEW_RESET_PHYSICAL, program,
            sizeof(program)) != TYPE_STATUS_OK;

    if (!failed) {
        machine->executor_cpu.data.eflags |= VCPU_EFLAGS_ZF;
        if (core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            machine->executor_cpu.data.eip != 0xfff9u) {
            failed = 1;
        } else if (core_machine_capture_observation(machine, &before) !=
            TYPE_STATUS_OK || !core_machine_cpu_execution_preview_lexeme(
                &machine->executor_cpu_execution, &lexeme) || !lexeme.available ||
            lexeme.byte_count != 1u || lexeme.component_count != 1u ||
            core_machine_capture_observation(machine, &after) != TYPE_STATUS_OK ||
            STD_MEMCMP(&before, &after, sizeof(before)) != 0) {
            failed = 1;
        }
    }
    core_machine_destroy(machine);
    return failed;
}
static C_INT preview_test_default_reset_alias(C_VOID)
{
    static const type_unsigned_8 halt[] = { 0xf4u };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x000ffff0u, halt,
            sizeof(halt)) != TYPE_STATUS_OK ||
        core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        result.executed != 1u ||
        machine->executor_cpu.data.eip != 0xfff1u;

    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    if (!preview_test_layouts()) return 2;
    if (!preview_test_unavailable()) return 3;
    if (preview_test_cpu_fetch_nonpublication()) return 4;
    if (preview_test_limited_fetch_nonpublication()) return 5;
    if (preview_test_taken_jcc_target()) return 6;
    if (preview_test_taken_near_jcc_target()) return 7;
    if (preview_test_default_reset_alias()) return 8;
    STD_PRINTF("M5:T357:S2:CPU-TIMING-PREVIEW:OK\n");
    return 0;
}
