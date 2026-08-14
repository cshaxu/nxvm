#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TIMING_8086_RESET_LINEAR 0xfffffff0u
#define TIMING_8086_RESET_PHYSICAL 0x000ffff0u
#define TIMING_8086_WINDOW_BYTES 64u

typedef struct timing_8086_state {
    type_unsigned_32 reads;
    type_unsigned_32 writes;
    type_unsigned_64 advanced_ticks;
} timing_8086_state;

static type_status timing_8086_port_read(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    timing_8086_state *state = (timing_8086_state *)owner;

    if (state == STD_NULL || out_value == STD_NULL || port != 0x0080u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ++state->reads;
    *out_value = 0x5au;
    return TYPE_STATUS_OK;
}

static type_status timing_8086_port_write(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 value)
{
    timing_8086_state *state = (timing_8086_state *)owner;

    if (state == STD_NULL || port != 0x0080u || value > 0xffffu) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ++state->writes;
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider timing_8086_port_provider = {
    timing_8086_port_read,
    timing_8086_port_write
};

static C_VOID timing_8086_execution_reset(C_VOID *opaque)
{
    timing_8086_state *state = (timing_8086_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks = 0u;
}

static C_VOID timing_8086_execution_advance(C_VOID *opaque,
    type_unsigned_64 elapsed_ticks)
{
    timing_8086_state *state = (timing_8086_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks += elapsed_ticks;
}

static const core_machine_execution_provider timing_8086_execution_provider = {
    timing_8086_execution_reset,
    STD_NULL,
    timing_8086_execution_advance
};

static C_INT timing_8086_prepare(core_machine **out_machine,
    timing_8086_state *state)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .ticks_per_instruction = 29u,
        .instruction_timing = { 29u, 7u, 31u, 37u, 41u, 43u }
    };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || state == STD_NULL ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_8086_RESET_LINEAR, TIMING_8086_RESET_PHYSICAL,
            TIMING_8086_WINDOW_BYTES) != TYPE_STATUS_OK ||
        core_machine_install_port_provider(machine, 0x0080u, 0x0080u,
            &timing_8086_port_provider, state) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &timing_8086_execution_provider, state)) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static C_INT timing_8086_load(core_machine *machine,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes)
{
    return machine != STD_NULL && program != STD_NULL &&
        core_machine_reset(machine) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, TIMING_8086_RESET_LINEAR, program,
            program_bytes) == TYPE_STATUS_OK;
}

static C_INT timing_8086_execute(core_machine *machine,
    type_unsigned_64 instructions, type_unsigned_64 expected_ticks,
    timing_8086_state *state)
{
    const core_machine_run_budget budget = { instructions, 0u };
    core_machine_run_result result;

    return machine != STD_NULL && state != STD_NULL &&
        core_machine_run(machine, budget, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        result.executed == instructions && result.ticks == expected_ticks &&
        result.elapsed_ticks == expected_ticks && state->advanced_ticks == expected_ticks;
}

static C_INT timing_8086_case(const type_unsigned_8 *program,
    STD_SIZE_T program_bytes, type_unsigned_64 instructions,
    type_unsigned_64 expected_ticks)
{
    timing_8086_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_8086_prepare(&machine, &state) ||
        !timing_8086_load(machine, program, program_bytes) ||
        !timing_8086_execute(machine, instructions, expected_ticks, &state);

    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_8086_test_baseline(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 clc[] = { 0xf8u };
    static const type_unsigned_8 mov_immediate[] = { 0xb8u, 0x34u, 0x12u };
    static const type_unsigned_8 mov_register[] = { 0x8bu, 0xc1u };

    return !timing_8086_case(nop, sizeof(nop), 1u, 3u) &&
        !timing_8086_case(clc, sizeof(clc), 1u, 2u) &&
        !timing_8086_case(mov_immediate, sizeof(mov_immediate), 1u, 4u) &&
        !timing_8086_case(mov_register, sizeof(mov_register), 1u, 2u);
}

static C_INT timing_8086_test_memory(C_VOID)
{
    static const type_unsigned_8 read_direct[] = { 0x8bu, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 write_direct[] = { 0x89u, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 read_odd[] = { 0x8bu, 0x0eu, 0x01u, 0x10u };
    static const type_unsigned_8 read_override[] = { 0x26u, 0x8bu, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 read_indexed[] = { 0x8bu, 0x8bu, 0x00u, 0x10u };
    static const type_unsigned_8 moffs_read_odd[] = { 0xa1u, 0x01u, 0x10u };
    static const type_unsigned_8 moffs_write_odd[] = { 0xa3u, 0x01u, 0x10u };
    const type_unsigned_16 value = 0x5aa5u;
    timing_8086_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_8086_prepare(&machine, &state);

    if (!failed) {
        failed |= !timing_8086_load(machine, read_direct, sizeof(read_direct)) ||
            core_machine_memory_write(machine, 0x1000u, &value, sizeof(value)) !=
                TYPE_STATUS_OK || !timing_8086_execute(machine, 1u, 14u, &state) ||
            machine->executor_cpu.data.cx != value;
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, write_direct, sizeof(write_direct)) ||
            ((machine->executor_cpu.data.cx = value), 0) ||
            !timing_8086_execute(machine, 1u, 15u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, read_odd, sizeof(read_odd)) ||
            core_machine_memory_write(machine, 0x1001u, &value, sizeof(value)) !=
                TYPE_STATUS_OK || !timing_8086_execute(machine, 1u, 18u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, read_override,
            sizeof(read_override)) || core_machine_memory_write(machine, 0x1000u,
                &value, sizeof(value)) != TYPE_STATUS_OK ||
            !timing_8086_execute(machine, 1u, 16u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, read_indexed, sizeof(read_indexed)) ||
            ((machine->executor_cpu.data.bp = 0u),
                (machine->executor_cpu.data.di = 0u), 0) ||
            core_machine_memory_write(machine, 0x1000u, &value, sizeof(value)) !=
                TYPE_STATUS_OK || !timing_8086_execute(machine, 1u, 20u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, moffs_read_odd,
            sizeof(moffs_read_odd)) || core_machine_memory_write(machine, 0x1001u,
                &value, sizeof(value)) != TYPE_STATUS_OK ||
            !timing_8086_execute(machine, 1u, 14u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, moffs_write_odd,
            sizeof(moffs_write_odd)) || ((machine->executor_cpu.data.ax = value), 0) ||
            !timing_8086_execute(machine, 1u, 14u, &state);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_8086_test_control_repeat_and_ports(C_VOID)
{
    static const type_unsigned_8 taken[] = { 0x74u, 0x01u, 0x90u, 0x90u };
    static const type_unsigned_8 not_taken[] = { 0x75u, 0x01u, 0x90u, 0x90u };
    static const type_unsigned_8 movsb[] = { 0xa4u };
    static const type_unsigned_8 rep_movsb[] = { 0xf3u, 0xa4u };
    static const type_unsigned_8 source[] = { 0x11u, 0x22u, 0x33u };
    static const type_unsigned_8 in_immediate[] = { 0xe4u, 0x80u };
    static const type_unsigned_8 in_dx[] = { 0xecu };
    static const type_unsigned_8 out_immediate[] = { 0xe6u, 0x80u };
    static const type_unsigned_8 out_dx[] = { 0xeeu };
    timing_8086_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_8086_prepare(&machine, &state);

    if (!failed) {
        failed |= !timing_8086_load(machine, taken, sizeof(taken)) ||
            ((machine->executor_cpu.data.eflags |= VCPU_EFLAGS_ZF), 0) ||
            !timing_8086_execute(machine, 1u, 16u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, not_taken, sizeof(not_taken)) ||
            ((machine->executor_cpu.data.eflags |= VCPU_EFLAGS_ZF), 0) ||
            !timing_8086_execute(machine, 1u, 4u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, movsb, sizeof(movsb)) ||
            !timing_8086_execute(machine, 1u, 18u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, rep_movsb, sizeof(rep_movsb)) ||
            core_machine_memory_write(machine, 0x1000u, source, sizeof(source)) !=
                TYPE_STATUS_OK || ((machine->executor_cpu.data.cx = 3u),
                (machine->executor_cpu.data.si = 0x1000u),
                (machine->executor_cpu.data.di = 0x1100u), 0) ||
            !timing_8086_execute(machine, 3u, 60u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, in_immediate, sizeof(in_immediate)) ||
            !timing_8086_execute(machine, 1u, 10u, &state) || state.reads != 1u;
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, in_dx, sizeof(in_dx)) ||
            ((machine->executor_cpu.data.dx = 0x0080u), 0) ||
            !timing_8086_execute(machine, 1u, 8u, &state) || state.reads != 2u;
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, out_immediate,
            sizeof(out_immediate)) || !timing_8086_execute(machine, 1u, 10u,
                &state) || state.writes != 1u;
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, out_dx, sizeof(out_dx)) ||
            ((machine->executor_cpu.data.dx = 0x0080u), 0) ||
            !timing_8086_execute(machine, 1u, 8u, &state) || state.writes != 2u;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_8086_test_fallback_fault_budget_and_overflow(C_VOID)
{
    static const type_unsigned_8 unallocated[] = { 0x31u, 0xc0u };
    static const type_unsigned_8 fault[] = { 0x66u, 0x90u };
    static const type_unsigned_8 maximum[] = { 0x26u, 0x89u, 0x8bu, 0x00u, 0x10u };
    static const type_unsigned_8 nop[] = { 0x90u };
    const core_machine_run_budget one = { 1u, 0u };
    const core_machine_run_budget insufficient = { 1u, 26u };
    const core_machine_run_budget sufficient = { 1u, 27u };
    core_machine_run_result result;
    timing_8086_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_8086_prepare(&machine, &state);

    if (!failed) {
        failed |= !timing_8086_load(machine, unallocated, sizeof(unallocated)) ||
            !timing_8086_execute(machine, 1u, 1u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, nop, sizeof(nop)) ||
            !timing_8086_execute(machine, 1u, 3u, &state) ||
            core_machine_reset(machine) != TYPE_STATUS_OK ||
            machine->elapsed_ticks != 0u || state.advanced_ticks != 0u ||
            !timing_8086_load(machine, nop, sizeof(nop)) ||
            !timing_8086_execute(machine, 1u, 3u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, nop, sizeof(nop)) ||
            core_machine_request_stop(machine) != TYPE_STATUS_OK ||
            core_machine_run(machine, one, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_REQUESTED || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u ||
            state.advanced_ticks != 0u;
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, fault, sizeof(fault)) ||
            !test_core_machine_fixture_preflight_real_ud_terminal(machine) ||
            core_machine_run(machine, one, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u ||
            state.advanced_ticks != 0u;
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, maximum, sizeof(maximum)) ||
            ((machine->executor_cpu.data.bp = 1u),
                (machine->executor_cpu.data.di = 0u),
                (machine->executor_cpu.data.cx = 0x5aa5u), 0) ||
            core_machine_run(machine, insufficient, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
            result.ticks != 0u || !timing_8086_execute(machine, 1u, 27u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, nop, sizeof(nop));
        machine->elapsed_ticks = UINT64_MAX - 2u;
        state.advanced_ticks = 0u;
        failed |= core_machine_run(machine, sufficient, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != UINT64_MAX - 2u ||
            machine->elapsed_ticks != UINT64_MAX - 2u || state.advanced_ticks != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    if (!timing_8086_test_baseline()) return 1;
    if (timing_8086_test_memory()) return 2;
    if (timing_8086_test_control_repeat_and_ports()) return 3;
    if (timing_8086_test_fallback_fault_budget_and_overflow()) return 4;
    STD_PRINTF("M5:T357:S4:8086-INSTRUCTION-TIMING-LEDGER:OK\n");
    return 0;
}
