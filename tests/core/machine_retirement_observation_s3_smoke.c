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
    core_machine_retirement_timing_origin expected_origin,
    const type_unsigned_8 *program, STD_SIZE_T bytes, C_INT expected_repeat)
{
    const core_machine_config physical = {
        .cpu_profile = profile,
        .time_axis = { CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u },
        .retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_retirement_observation_provider provider;
    core_machine_run_result result;
    core_machine_timeline_observation timeline;
    retirement_probe probe = { STD_NULL, { { 0 } }, 0u, TYPE_STATUS_OK };
    core_machine *machine = STD_NULL;
    C_INT failed = !retirement_prepare(&machine, &physical, program, bytes);

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
            (expected_repeat && (probe.records[0].formula_inputs &
                CORE_MACHINE_CPU_TIMING_INPUT_REPEAT) == 0u) ||
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

static C_INT retirement_8088_primary_case(const type_unsigned_8 *program,
    STD_SIZE_T bytes, type_unsigned_64 expected_ticks,
    core_machine_retirement_timing_origin expected_origin)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8088
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_16 value = 1u;
    core_machine_retirement_observation_provider provider;
    core_machine_run_result result;
    retirement_probe probe = { STD_NULL, { { 0 } }, 0u, TYPE_STATUS_OK };
    core_machine *machine = STD_NULL;
    C_INT failed = !retirement_prepare(&machine, &config, program, bytes);

    provider.callback = retirement_capture;
    provider.context = &probe;
    probe.machine = machine;
    if (!failed) {
        failed |= core_machine_memory_write(machine, 0x1000u, &value,
                sizeof(value)) != TYPE_STATUS_OK ||
            core_machine_set_retirement_observation_provider(machine,
                &provider) != TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.executed != 1u || probe.count != 1u ||
            probe.records[0].source_ticks != expected_ticks ||
            probe.records[0].timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            probe.records[0].timing_origin !=
                expected_origin;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT retirement_8088_branch_case(const type_unsigned_8 *program,
    STD_SIZE_T bytes, type_unsigned_16 count, type_unsigned_32 flags,
    type_unsigned_64 expected_ticks)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8088
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_retirement_observation_provider provider;
    core_machine_run_result result;
    retirement_probe probe = { STD_NULL, { { 0 } }, 0u, TYPE_STATUS_OK };
    core_machine *machine = STD_NULL;
    C_INT failed = !retirement_prepare(&machine, &config, program, bytes);

    provider.callback = retirement_capture;
    provider.context = &probe;
    probe.machine = machine;
    if (!failed) {
        machine->executor_cpu.data.cx = count;
        machine->executor_cpu.data.eflags = flags;
        failed |= core_machine_set_retirement_observation_provider(machine,
                &provider) != TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.executed != 1u || probe.count != 1u ||
            probe.records[0].source_ticks != expected_ticks ||
            probe.records[0].timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            probe.records[0].timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            probe.records[0].source_timing_form_id ==
                CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT retirement_8088_jcc_forms_case(C_VOID)
{
    static const type_unsigned_32 flags[16][2] = {
        { VCPU_EFLAGS_OF, 0u }, { 0u, VCPU_EFLAGS_OF },
        { VCPU_EFLAGS_CF, 0u }, { 0u, VCPU_EFLAGS_CF },
        { VCPU_EFLAGS_ZF, 0u }, { 0u, VCPU_EFLAGS_ZF },
        { VCPU_EFLAGS_CF, 0u }, { 0u, VCPU_EFLAGS_CF },
        { VCPU_EFLAGS_SF, 0u }, { 0u, VCPU_EFLAGS_SF },
        { VCPU_EFLAGS_PF, 0u }, { 0u, VCPU_EFLAGS_PF },
        { VCPU_EFLAGS_SF, 0u }, { 0u, VCPU_EFLAGS_SF },
        { VCPU_EFLAGS_ZF, 0u }, { 0u, VCPU_EFLAGS_ZF }
    };
    type_unsigned_8 opcode;
    C_INT failed = 0;

    for (opcode = 0x70u; opcode <= 0x7fu; ++opcode) {
        const type_unsigned_8 program[] = { opcode, 0x01u };
        type_unsigned_8 index = opcode - 0x70u;

        failed |= retirement_8088_branch_case(program, sizeof(program), 0u,
            flags[index][0], 16u) || retirement_8088_branch_case(program,
            sizeof(program), 0u, flags[index][1], 4u);
    }
    return failed;
}

static C_INT retirement_8088_string_case(const type_unsigned_8 *program,
    STD_SIZE_T bytes, type_unsigned_16 count, type_unsigned_32 executions,
    type_unsigned_64 first_ticks, type_unsigned_64 next_ticks)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8088
    };
    const core_machine_run_budget budget = { executions, 0u };
    core_machine_retirement_observation_provider provider;
    core_machine_run_result result;
    retirement_probe probe = { STD_NULL, { { 0 } }, 0u, TYPE_STATUS_OK };
    core_machine *machine = STD_NULL;
    C_INT failed = !retirement_prepare(&machine, &config, program, bytes);

    provider.callback = retirement_capture;
    provider.context = &probe;
    probe.machine = machine;
    if (!failed) {
        machine->executor_cpu.data.cx = count;
        failed |= core_machine_set_retirement_observation_provider(machine,
                &provider) != TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.executed != executions || probe.count != executions ||
            probe.records[0].source_ticks != first_ticks ||
            probe.records[0].timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            probe.records[0].timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO;
        if (executions > 1u) {
            failed |= probe.records[1].source_ticks != next_ticks ||
                probe.records[0].repeat_phase !=
                    CORE_MACHINE_RETIREMENT_REPEAT_FIRST ||
                probe.records[1].repeat_phase !=
                    CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION;
        }
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
    const type_unsigned_8 add_register[] = { 0x01u, 0xc8u };
    const type_unsigned_8 add_register_memory[] = { 0x03u, 0x06u, 0x00u, 0x10u };
    const type_unsigned_8 add_memory_register[] = { 0x01u, 0x06u, 0x00u, 0x10u };
    const type_unsigned_8 push_register[] = { 0x50u };
    const type_unsigned_8 pop_register[] = { 0x58u };
    const type_unsigned_8 call_near[] = { 0xe8u, 0x00u, 0x00u };
    const type_unsigned_8 ret_near[] = { 0xc3u };
    const type_unsigned_8 ret_far[] = { 0xcbu };
    const type_unsigned_8 call_memory[] = { 0xffu, 0x16u, 0x00u, 0x10u };
    const type_unsigned_8 push_memory[] = { 0xffu, 0x36u, 0x00u, 0x10u };
    const type_unsigned_8 pop_memory[] = { 0x8fu, 0x06u, 0x00u, 0x10u };
    const type_unsigned_8 in_immediate_byte[] = { 0xe4u, 0x00u };
    const type_unsigned_8 in_immediate_word[] = { 0xe5u, 0x00u };
    const type_unsigned_8 in_dx_byte[] = { 0xecu };
    const type_unsigned_8 in_dx_word[] = { 0xedu };
    const type_unsigned_8 out_immediate_byte[] = { 0xe6u, 0x00u };
    const type_unsigned_8 out_immediate_word[] = { 0xe7u, 0x00u };
    const type_unsigned_8 out_dx_byte[] = { 0xeeu };
    const type_unsigned_8 out_dx_word[] = { 0xefu };
    const type_unsigned_8 jcxz[] = { 0xe3u, 0x01u };
    const type_unsigned_8 loop[] = { 0xe2u, 0x01u };
    const type_unsigned_8 loope[] = { 0xe1u, 0x01u };
    const type_unsigned_8 loopne[] = { 0xe0u, 0x01u };
    const type_unsigned_8 int3[] = { 0xccu };
    const type_unsigned_8 hlt[] = { 0xf4u };
    const type_unsigned_8 movsb[] = { 0xa4u };
    const type_unsigned_8 movsw[] = { 0xa5u };
    const type_unsigned_8 cmpsb[] = { 0xa6u };
    const type_unsigned_8 cmpsw[] = { 0xa7u };
    const type_unsigned_8 stosb[] = { 0xaau };
    const type_unsigned_8 stosw[] = { 0xabu };
    const type_unsigned_8 lodsb[] = { 0xacu };
    const type_unsigned_8 lodsw[] = { 0xadu };
    const type_unsigned_8 scasb[] = { 0xaeu };
    const type_unsigned_8 scasw[] = { 0xafu };
    const type_unsigned_8 segment_movsw[] = { 0x26u, 0xa5u };
    const type_unsigned_8 rep_movsw[] = { 0xf3u, 0xa5u };
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
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY, rep_nop,
        sizeof(rep_nop), 1) ||
        retirement_unallocated_profile_case(CORE_MACHINE_CPU_PROFILE_8088,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_COMPATIBILITY, rep_nop,
            sizeof(rep_nop), 1) ||
        retirement_unallocated_profile_case(CORE_MACHINE_CPU_PROFILE_80186,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK, rep_nop,
            sizeof(rep_nop), 1) ||
        retirement_unallocated_profile_case(CORE_MACHINE_CPU_PROFILE_80286,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80286_FALLBACK, rep_nop,
            sizeof(rep_nop), 1) ||
        retirement_unallocated_profile_case(CORE_MACHINE_CPU_PROFILE_80386,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_FALLBACK, rep_nop,
            sizeof(rep_nop), 1) ||
        retirement_unallocated_profile_case(CORE_MACHINE_CPU_PROFILE_8088,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK, int3,
            sizeof(int3), 0) ||
        retirement_unallocated_profile_case(CORE_MACHINE_CPU_PROFILE_8088,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK, hlt,
            sizeof(hlt), 0);
    failed |= retirement_control_context_case(0x75u,
        CORE_MACHINE_RETIREMENT_CONTROL_TAKEN, 1u) ||
        retirement_control_context_case(0x74u,
            CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH,
            CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE) ||
        retirement_pre_mode_snapshot_case() || retirement_8086_context_formula_case() ||
        retirement_8088_primary_case(add_register, sizeof(add_register), 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(add_register_memory,
            sizeof(add_register_memory), 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(add_memory_register,
            sizeof(add_memory_register), 30u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(push_register, sizeof(push_register), 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(pop_register, sizeof(pop_register), 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(call_near, sizeof(call_near), 23u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(ret_near, sizeof(ret_near), 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(ret_far, sizeof(ret_far), 26u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(call_memory, sizeof(call_memory), 35u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(push_memory, sizeof(push_memory), 30u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(pop_memory, sizeof(pop_memory), 31u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(in_immediate_byte,
            sizeof(in_immediate_byte), 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_primary_case(in_immediate_word,
            sizeof(in_immediate_word), 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_primary_case(in_dx_byte, sizeof(in_dx_byte), 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_primary_case(in_dx_word, sizeof(in_dx_word), 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_primary_case(out_immediate_byte,
            sizeof(out_immediate_byte), 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_primary_case(out_immediate_word,
            sizeof(out_immediate_word), 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_primary_case(out_dx_byte, sizeof(out_dx_byte), 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_primary_case(out_dx_word, sizeof(out_dx_word), 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_jcc_forms_case() ||
        retirement_8088_branch_case(jcxz, sizeof(jcxz), 0u, 0u, 18u) ||
        retirement_8088_branch_case(jcxz, sizeof(jcxz), 1u, 0u, 6u) ||
        retirement_8088_branch_case(loop, sizeof(loop), 2u, 0u, 17u) ||
        retirement_8088_branch_case(loop, sizeof(loop), 1u, 0u, 5u) ||
        retirement_8088_branch_case(loope, sizeof(loope), 2u,
            VCPU_EFLAGS_ZF, 18u) ||
        retirement_8088_branch_case(loope, sizeof(loope), 1u,
            VCPU_EFLAGS_ZF, 6u) ||
        retirement_8088_branch_case(loopne, sizeof(loopne), 2u, 0u, 19u) ||
        retirement_8088_branch_case(loopne, sizeof(loopne), 1u, 0u, 5u) ||
        retirement_8088_string_case(movsb, sizeof(movsb), 0u, 1u, 18u, 0u) ||
        retirement_8088_string_case(movsw, sizeof(movsw), 0u, 1u, 26u, 0u) ||
        retirement_8088_string_case(cmpsb, sizeof(cmpsb), 0u, 1u, 22u, 0u) ||
        retirement_8088_string_case(cmpsw, sizeof(cmpsw), 0u, 1u, 30u, 0u) ||
        retirement_8088_string_case(stosb, sizeof(stosb), 0u, 1u, 11u, 0u) ||
        retirement_8088_string_case(stosw, sizeof(stosw), 0u, 1u, 15u, 0u) ||
        retirement_8088_string_case(lodsb, sizeof(lodsb), 0u, 1u, 12u, 0u) ||
        retirement_8088_string_case(lodsw, sizeof(lodsw), 0u, 1u, 16u, 0u) ||
        retirement_8088_string_case(scasb, sizeof(scasb), 0u, 1u, 15u, 0u) ||
        retirement_8088_string_case(scasw, sizeof(scasw), 0u, 1u, 19u, 0u) ||
        retirement_8088_string_case(segment_movsw, sizeof(segment_movsw), 0u,
            1u, 28u, 0u) ||
        retirement_8088_string_case(rep_movsw, sizeof(rep_movsw), 2u, 2u,
            34u, 25u) ||
        retirement_8088_string_case(rep_movsw, sizeof(rep_movsw), 0u, 1u,
            9u, 0u);
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
