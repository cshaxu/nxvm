#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/cpu_timing.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct retirement_probe {
    core_machine *machine;
    core_machine_retirement_observation records[3];
    type_unsigned_32 count;
    type_status set_while_running;
} retirement_probe;

static C_VOID retirement_capture(C_VOID *opaque,
    const core_machine_retirement_observation *observation)
{
    retirement_probe *probe = (retirement_probe *)opaque;

    if (probe == STD_NULL || observation == STD_NULL) return;
    if (probe->count < 3u) probe->records[probe->count] = *observation;
    ++probe->count;
    probe->set_while_running = core_machine_set_retirement_observation_provider(
        probe->machine, STD_NULL);
}

static C_INT retirement_prepare(core_machine **out_machine,
    const core_machine_config *config, const type_unsigned_8 *code,
    STD_SIZE_T bytes)
{
    return out_machine != STD_NULL &&
        core_machine_create(config, out_machine) == TYPE_STATUS_OK &&
        test_core_machine_fixture_register_reset_mapping(*out_machine, 0xfffffff0u,
            0x000ffff0u, 16u) == TYPE_STATUS_OK &&
        core_machine_freeze_execution_providers(*out_machine) == TYPE_STATUS_OK &&
        core_machine_reset(*out_machine) == TYPE_STATUS_OK &&
        core_machine_set_a20(*out_machine, 1) == TYPE_STATUS_OK &&
        core_machine_memory_write(*out_machine, 0xfffffff0u, code, bytes) ==
            TYPE_STATUS_OK;
}

static C_INT retirement_control_context_case(type_unsigned_8 opcode,
    core_machine_retirement_control_outcome expected_outcome,
    type_unsigned_8 expected_next_components)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { opcode, 0x01u, 0x90u, 0x90u };
    core_machine_retirement_observation_provider provider;
    core_machine_run_result result;
    retirement_probe probe = { STD_NULL, { { 0 } }, 0u, TYPE_STATUS_OK };
    core_machine *machine = STD_NULL;
    C_INT failed = !retirement_prepare(&machine, &config, program, sizeof(program));

    provider.callback = retirement_capture;
    provider.context = &probe;
    probe.machine = machine;
    if (!failed) {
        failed |= core_machine_set_retirement_observation_provider(machine,
            &provider) != TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            probe.count != 1u ||
            probe.records[0].control_outcome != expected_outcome ||
            probe.records[0].next_lexeme_components != expected_next_components;
    }
    core_machine_destroy(machine);
    return failed;
}
static C_INT retirement_pre_mode_snapshot_case(C_VOID)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0x0fu, 0x01u, 0xf0u };
    core_machine_retirement_observation_provider provider;
    core_machine_run_result result;
    retirement_probe probe = { STD_NULL, { { 0 } }, 0u, TYPE_STATUS_OK };
    core_machine *machine = STD_NULL;
    C_INT failed = !retirement_prepare(&machine, &config, program, sizeof(program));

    provider.callback = retirement_capture;
    provider.context = &probe;
    probe.machine = machine;
    if (!failed) {
        machine->executor_cpu.data.eax = 1u;
        failed |= core_machine_set_retirement_observation_provider(machine,
            &provider) != TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            probe.count != 1u || probe.records[0].protected_mode ||
            probe.records[0].eligibility_key.protected_mode ||
            (machine->executor_cpu.data.cr0 & VCPU_CR0_PE) == 0u;
    }
    core_machine_destroy(machine);
    return failed;
}
static C_INT retirement_unallocated_profile_case(core_machine_cpu_profile profile,
    core_machine_retirement_timing_origin expected_origin)
{
    const core_machine_config physical = {
        .cpu_profile = profile,
        .time_axis = { CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u },
        .retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 rep_nop[] = { 0xf3u, 0x90u };
    core_machine_retirement_observation_provider provider;
    core_machine_run_result result;
    core_machine_timeline_observation timeline;
    retirement_probe probe = { STD_NULL, { { 0 } }, 0u, TYPE_STATUS_OK };
    core_machine *machine = STD_NULL;
    C_INT failed = !retirement_prepare(&machine, &physical, rep_nop, sizeof(rep_nop));

    provider.callback = retirement_capture;
    provider.context = &probe;
    probe.machine = machine;
    if (!failed) {
        failed |= core_machine_set_retirement_observation_provider(machine,
            &provider) != TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            probe.count != 1u || probe.set_while_running != TYPE_STATUS_INVALID_STATE ||
            probe.records[0].timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED ||
            probe.records[0].source_timing_form_id !=
                CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
            probe.records[0].timing_key_id !=
                CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
            (probe.records[0].formula_inputs &
                CORE_MACHINE_CPU_TIMING_INPUT_REPEAT) == 0u ||
            probe.records[0].timing_origin != expected_origin ||
            probe.records[0].elapsed_ticks != 0u || probe.records[0].timeline_ticks != 0u ||
            core_machine_get_timeline_observation(machine, &timeline) != TYPE_STATUS_OK ||
            timeline.now != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT retirement_8086_context_formula_case(C_VOID)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 segment_movsb[] = { 0x26u, 0xa4u };
    const type_unsigned_8 lock_add[] = { 0xf0u, 0x01u, 0x06u, 0x00u, 0x10u };
    const type_unsigned_8 wait[] = { 0x9bu };
    const type_unsigned_16 value = 0u;
    core_machine_retirement_observation_provider provider;
    core_machine_run_result result;
    retirement_probe probe = { STD_NULL, { { 0 } }, 0u, TYPE_STATUS_OK };
    core_machine *machine = STD_NULL;
    C_INT failed = !retirement_prepare(&machine, &config, segment_movsb,
        sizeof(segment_movsb));

    provider.callback = retirement_capture;
    provider.context = &probe;
    probe.machine = machine;
    if (!failed) {
        failed |= core_machine_set_retirement_observation_provider(machine,
            &provider) != TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.executed != 1u || probe.count != 1u ||
            probe.records[0].source_ticks != 20u ||
            probe.records[0].timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            probe.records[0].timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO ||
            (probe.records[0].formula_inputs &
                CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE) == 0u;
    }
    if (!failed) {
        failed |= core_machine_reset(machine) != TYPE_STATUS_OK ||
            core_machine_set_a20(machine, 1) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, 0xfffffff0u, lock_add,
                sizeof(lock_add)) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, 0x1000u, &value,
                sizeof(value)) != TYPE_STATUS_OK ||
            ((machine->executor_cpu.data.ax = 1u), 0) ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.executed != 1u || probe.count != 2u ||
            probe.records[1].source_ticks != 24u ||
            probe.records[1].timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            probe.records[1].timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY ||
            probe.records[1].modrm_form != CORE_MACHINE_RETIREMENT_MODRM_MEMORY ||
            probe.records[1].modrm_extension != 0u ||
            (probe.records[1].formula_inputs &
                (CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
                CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
                CORE_MACHINE_CPU_TIMING_INPUT_LOCK)) !=
                (CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
                CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
                CORE_MACHINE_CPU_TIMING_INPUT_LOCK);
    }
    if (!failed) {
        failed |= core_machine_reset(machine) != TYPE_STATUS_OK ||
            core_machine_set_a20(machine, 1) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, 0xfffffff0u, wait,
                sizeof(wait)) != TYPE_STATUS_OK ||
            ((machine->fpu.wait_iterations = 3u), 0) ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.executed != 1u || probe.count != 3u ||
            probe.records[2].source_ticks != 18u ||
            probe.records[2].timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            probe.records[2].timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY ||
            (probe.records[2].formula_inputs &
                CORE_MACHINE_CPU_TIMING_INPUT_WAIT_ITERATIONS) == 0u;
    }
    core_machine_destroy(machine);
    return failed;
}
C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_config deterministic = { .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386 };
    core_machine_config physical = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .time_axis = { CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u },
        .retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL
    };
    core_machine_retirement_observation_provider provider;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_timeline_observation timeline;
    retirement_probe probe = { STD_NULL, { { 0 } }, 0u, TYPE_STATUS_OK };
    type_unsigned_8 nop = 0x90u;
    type_unsigned_8 rep_nop[] = { 0xf3u, 0x90u };
    C_INT failed = 0;

    provider.callback = retirement_capture;
    provider.context = &probe;
    failed |= !retirement_prepare(&machine, &deterministic, &nop, 1u);
    probe.machine = machine;
    failed |= core_machine_set_retirement_observation_provider(machine,
        &provider) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_set_a20(machine, 1) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        TYPE_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        probe.count != 1u || probe.set_while_running != TYPE_STATUS_INVALID_STATE ||
        probe.records[0].sequence != 0u ||
        probe.records[0].point.linear_pc != 0xfffffff0u ||
        probe.records[0].point.byte_count != CORE_MACHINE_CPU_DIAGNOSTIC_BYTES ||
        probe.records[0].point.bytes[0] != nop ||
        probe.records[0].cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        probe.records[0].timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        probe.records[0].source_timing_form_id != 0u ||
        probe.records[0].timing_key_id != 0u ||
        probe.records[0].formula_inputs != 0u ||
        probe.records[0].timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_FALLBACK ||
        probe.records[0].modrm_form != CORE_MACHINE_RETIREMENT_MODRM_UNAVAILABLE ||
        probe.records[0].modrm_extension !=
            CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE ||
        probe.records[0].control_outcome != CORE_MACHINE_RETIREMENT_CONTROL_NONE ||
        probe.records[0].next_lexeme_components !=
            CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE ||
        probe.records[0].repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_NONE ||
        probe.records[0].elapsed_ticks != 0u || probe.records[0].timeline_ticks != 0u ||
        probe.records[0].source_ticks == 0u || probe.records[0].protected_mode ||
        probe.records[0].virtual_8086_mode || probe.records[0].operand_size_32 ||
        probe.records[0].address_size_32 || probe.records[0].lock_prefix ||
        probe.records[0].repeat_prefix != 0u;
    failed |= retirement_unallocated_profile_case(CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_unallocated_profile_case(CORE_MACHINE_CPU_PROFILE_8088,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_COMPATIBILITY) ||
        retirement_unallocated_profile_case(CORE_MACHINE_CPU_PROFILE_80186,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK) ||
        retirement_unallocated_profile_case(CORE_MACHINE_CPU_PROFILE_80286,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80286_FALLBACK) ||
        retirement_unallocated_profile_case(CORE_MACHINE_CPU_PROFILE_80386,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_FALLBACK);
    failed |= retirement_control_context_case(0x75u,
        CORE_MACHINE_RETIREMENT_CONTROL_TAKEN, 1u) ||
        retirement_control_context_case(0x74u,
            CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH,
            CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE) ||
        retirement_pre_mode_snapshot_case() || retirement_8086_context_formula_case();
    failed |= core_machine_set_retirement_observation_provider(machine, STD_NULL) !=
        TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_set_a20(machine, 1) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        TYPE_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || probe.count != 1u;
    core_machine_destroy(machine);
    machine = STD_NULL;

    probe.machine = STD_NULL;
    probe.count = 0u;
    probe.set_while_running = TYPE_STATUS_OK;
    failed |= !retirement_prepare(&machine, &physical, rep_nop, sizeof(rep_nop));
    probe.machine = machine;
    failed |= core_machine_set_retirement_observation_provider(machine,
        &provider) != TYPE_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_FAULT;
    failed |= result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
        probe.count != 1u || probe.set_while_running != TYPE_STATUS_INVALID_STATE ||
        probe.records[0].point.byte_count != CORE_MACHINE_CPU_DIAGNOSTIC_BYTES ||
        probe.records[0].point.bytes[0] != rep_nop[0] ||
        probe.records[0].point.bytes[1] != rep_nop[1] ||
        probe.records[0].timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED ||
        probe.records[0].source_timing_form_id !=
            CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
        probe.records[0].timing_key_id !=
            CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
        (probe.records[0].formula_inputs &
            CORE_MACHINE_CPU_TIMING_INPUT_REPEAT) == 0u ||
        probe.records[0].timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_FALLBACK ||
        probe.records[0].modrm_form != CORE_MACHINE_RETIREMENT_MODRM_UNAVAILABLE ||
        probe.records[0].modrm_extension !=
            CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE ||
        probe.records[0].control_outcome != CORE_MACHINE_RETIREMENT_CONTROL_NONE ||
        probe.records[0].next_lexeme_components !=
            CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE ||
        probe.records[0].repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_NONE ||
        probe.records[0].elapsed_ticks != 0u || probe.records[0].timeline_ticks != 0u;
    failed |= core_machine_get_timeline_observation(machine, &timeline) != TYPE_STATUS_OK ||
        timeline.now != 0u;
    core_machine_destroy(machine);

    if (failed) return 1;
    STD_PRINTF("M5:T390:S3:RETIREMENT-OBSERVATION:OK\n");
    return 0;
}
