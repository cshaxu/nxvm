#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TIMING_80286_RESET_LINEAR 0xfffffff0u
#define TIMING_80286_RESET_PHYSICAL 0x000ffff0u

typedef struct timing_80286_state {
    type_unsigned_32 reads;
    type_unsigned_32 writes;
    type_unsigned_64 advanced_ticks;
} timing_80286_state;

static type_status timing_80286_read(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    timing_80286_state *state = (timing_80286_state *)owner;

    if (state == STD_NULL || out_value == STD_NULL || port != 0x0080u)
        return TYPE_STATUS_INVALID_ARGUMENT;
    ++state->reads;
    *out_value = 0x5au;
    return TYPE_STATUS_OK;
}

static type_status timing_80286_write(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 value)
{
    timing_80286_state *state = (timing_80286_state *)owner;

    if (state == STD_NULL || port != 0x0080u || value > 0xffffu)
        return TYPE_STATUS_INVALID_ARGUMENT;
    ++state->writes;
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider timing_80286_ports = {
    timing_80286_read, timing_80286_write
};

static C_VOID timing_80286_reset(C_VOID *opaque)
{
    timing_80286_state *state = (timing_80286_state *)opaque;
    if (state != STD_NULL) state->advanced_ticks = 0u;
}

static C_VOID timing_80286_advance(C_VOID *opaque, type_unsigned_64 ticks)
{
    timing_80286_state *state = (timing_80286_state *)opaque;
    if (state != STD_NULL) state->advanced_ticks += ticks;
}

static const core_machine_execution_provider timing_80286_execution = {
    timing_80286_reset, STD_NULL, timing_80286_advance
};

static C_INT timing_80286_prepare(core_machine **out_machine,
    timing_80286_state *state)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .ticks_per_instruction = 29u,
        .instruction_timing = { 29u, 7u, 31u, 37u, 41u, 43u }
    };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || state == STD_NULL ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_80286_RESET_LINEAR, TIMING_80286_RESET_PHYSICAL, 64u) !=
            TYPE_STATUS_OK || core_machine_install_port_provider(machine,
            0x0080u, 0x0080u, &timing_80286_ports, state) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &timing_80286_execution, state)) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static C_INT timing_80286_load(core_machine *machine,
    const type_unsigned_8 *program, STD_SIZE_T bytes)
{
    return core_machine_reset(machine) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, TIMING_80286_RESET_LINEAR,
            program, bytes) == TYPE_STATUS_OK;
}

static C_INT timing_80286_run(core_machine *machine, timing_80286_state *state,
    type_unsigned_64 instructions, type_unsigned_64 ticks)
{
    const core_machine_run_budget budget = { instructions, 0u };
    core_machine_run_result result;

    return core_machine_run(machine, budget, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        result.executed == instructions && result.ticks == ticks &&
        result.elapsed_ticks == ticks && state->advanced_ticks == ticks;
}

static C_INT timing_80286_case(const type_unsigned_8 *program, STD_SIZE_T bytes,
    type_unsigned_64 ticks)
{
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_load(machine, program, bytes) ||
        !timing_80286_run(machine, &state, 1u, ticks);

    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_memory(C_VOID)
{
    static const type_unsigned_8 direct_read[] = { 0x8bu, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 direct_write[] = { 0x89u, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 indexed_odd_read[] = { 0x8bu, 0x4au, 0x01u };
    static const type_unsigned_8 moffs_read[] = { 0xa1u, 0x01u, 0x10u };
    static const type_unsigned_8 moffs_write[] = { 0xa3u, 0x01u, 0x10u };
    const type_unsigned_16 value = 0x5aa5u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, direct_read,
        sizeof(direct_read)) || core_machine_memory_write(machine, 0x1000u,
        &value, sizeof(value)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 5u) ||
        machine->executor_cpu.data.cx != value;
    if (!failed) failed |= !timing_80286_load(machine, direct_write,
        sizeof(direct_write)) || ((machine->executor_cpu.data.cx = value), 0) ||
        !timing_80286_run(machine, &state, 1u, 3u);
    if (!failed) failed |= !timing_80286_load(machine, indexed_odd_read,
        sizeof(indexed_odd_read)) || ((machine->executor_cpu.data.bp = 0x1000u),
        (machine->executor_cpu.data.si = 0u), 0) ||
        core_machine_memory_write(machine, 0x1001u, &value, sizeof(value)) !=
            TYPE_STATUS_OK || !timing_80286_run(machine, &state, 1u, 8u) ||
        machine->executor_cpu.data.cx != value;
    if (!failed) failed |= !timing_80286_load(machine, moffs_read,
        sizeof(moffs_read)) || core_machine_memory_write(machine, 0x1001u,
        &value, sizeof(value)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !timing_80286_load(machine, moffs_write,
        sizeof(moffs_write)) || ((machine->executor_cpu.data.ax = value), 0) ||
        !timing_80286_run(machine, &state, 1u, 5u);
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_control_ports(C_VOID)
{
    static const type_unsigned_8 taken[] = { 0x74u, 0x01u, 0x90u, 0x90u };
    static const type_unsigned_8 not_taken[] = { 0x75u, 0x01u, 0x90u, 0x90u };
    static const type_unsigned_8 movsb[] = { 0xa4u };
    static const type_unsigned_8 rep[] = { 0xf3u, 0xa4u };
    static const type_unsigned_8 source[] = { 1u, 2u, 3u };
    static const type_unsigned_8 out_imm[] = { 0xe6u, 0x80u };
    static const type_unsigned_8 out_dx[] = { 0xeeu };
    static const type_unsigned_8 in_imm[] = { 0xe4u, 0x80u };
    static const type_unsigned_8 in_dx[] = { 0xecu };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, taken, sizeof(taken)) ||
        ((machine->executor_cpu.data.eflags |= VCPU_EFLAGS_ZF), 0) ||
        !timing_80286_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !timing_80286_load(machine, not_taken,
        sizeof(not_taken)) || ((machine->executor_cpu.data.eflags |=
        VCPU_EFLAGS_ZF), 0) || !timing_80286_run(machine, &state, 1u, 3u);
    if (!failed) failed |= !timing_80286_load(machine, movsb, sizeof(movsb)) ||
        core_machine_memory_write(machine, 0x1000u, source, 1u) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.si = 0x1000u),
            (machine->executor_cpu.data.di = 0x1100u), 0) ||
        !timing_80286_run(machine, &state, 1u, 5u);
    if (!failed) failed |= !timing_80286_load(machine, rep, sizeof(rep)) ||
        core_machine_memory_write(machine, 0x1000u, source, sizeof(source)) !=
            TYPE_STATUS_OK || ((machine->executor_cpu.data.cx = 3u),
            (machine->executor_cpu.data.si = 0x1000u),
            (machine->executor_cpu.data.di = 0x1100u), 0) ||
        !timing_80286_run(machine, &state, 3u, 17u);
    if (!failed) failed |= !timing_80286_load(machine, out_imm, sizeof(out_imm)) ||
        !timing_80286_run(machine, &state, 1u, 3u) || state.writes != 1u;
    if (!failed) failed |= !timing_80286_load(machine, out_dx, sizeof(out_dx)) ||
        ((machine->executor_cpu.data.dx = 0x0080u), 0) ||
        !timing_80286_run(machine, &state, 1u, 3u) || state.writes != 2u;
    if (!failed) failed |= !timing_80286_load(machine, in_imm, sizeof(in_imm)) ||
        !timing_80286_run(machine, &state, 1u, 5u) || state.reads != 1u;
    if (!failed) failed |= !timing_80286_load(machine, in_dx, sizeof(in_dx)) ||
        ((machine->executor_cpu.data.dx = 0x0080u), 0) ||
        !timing_80286_run(machine, &state, 1u, 5u) || state.reads != 2u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_boundaries(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 shift_byte[] = { 0xd0u, 0xc0u };
    static const type_unsigned_8 shift_word[] = { 0xd1u, 0xc0u };
    static const type_unsigned_8 shift_memory[] = { 0xd0u, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 shift_undefined[] = { 0xd0u, 0xf0u };
    static const type_unsigned_8 fault[] = { 0x66u, 0x90u };
    static const type_unsigned_8 maximum[] = { 0xf3u, 0xa4u };
    static const type_unsigned_8 source[] = { 0x78u };
    const core_machine_run_budget one = { 1u, 0u };
    const core_machine_run_budget insufficient = { 1u, 8u };
    core_machine_run_result result;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, shift_byte,
        sizeof(shift_byte)) || !timing_80286_run(machine, &state, 1u, 2u);
    if (!failed) failed |= !timing_80286_load(machine, shift_word,
        sizeof(shift_word)) || !timing_80286_run(machine, &state, 1u, 2u);
    if (!failed) failed |= !timing_80286_load(machine, shift_memory,
        sizeof(shift_memory)) || !timing_80286_run(machine, &state, 1u, 1u);
    if (!failed) failed |= !timing_80286_load(machine, shift_undefined,
        sizeof(shift_undefined)) || !test_core_machine_fixture_preflight_real_ud_terminal(
            machine) || core_machine_run(machine, one, &result) != TYPE_STATUS_FAULT ||
        result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
        result.ticks != 0u || state.advanced_ticks != 0u;
    if (!failed) failed |= !timing_80286_load(machine, fault, sizeof(fault)) ||
        !test_core_machine_fixture_preflight_real_ud_terminal(machine) ||
        core_machine_run(machine, one, &result) != TYPE_STATUS_FAULT ||
        result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
        result.ticks != 0u || state.advanced_ticks != 0u;
    if (!failed) failed |= !timing_80286_load(machine, nop, sizeof(nop)) ||
        !timing_80286_run(machine, &state, 1u, 3u) ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        machine->elapsed_ticks != 0u || state.advanced_ticks != 0u ||
        !timing_80286_load(machine, nop, sizeof(nop)) ||
        core_machine_request_stop(machine) != TYPE_STATUS_OK ||
        core_machine_run(machine, one, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_REQUESTED || result.executed != 0u ||
        result.ticks != 0u || state.advanced_ticks != 0u;
    if (!failed) failed |= !timing_80286_load(machine, maximum,
        sizeof(maximum)) || ((machine->executor_cpu.data.cx = 1u),
        (machine->executor_cpu.data.si = 0x1000u),
        (machine->executor_cpu.data.di = 0x1100u), 0) ||
        core_machine_memory_write(machine, 0x1000u, source, sizeof(source)) !=
            TYPE_STATUS_OK ||
        core_machine_run(machine, insufficient, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
        result.ticks != 0u || !timing_80286_run(machine, &state, 1u, 9u);
    if (!failed) failed |= !timing_80286_load(machine, nop, sizeof(nop));
    if (!failed) {
        machine->elapsed_ticks = UINT64_MAX - 2u;
        state.advanced_ticks = 0u;
        failed |= core_machine_run(machine, one, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || machine->elapsed_ticks != UINT64_MAX - 2u ||
            state.advanced_ticks != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 clc[] = { 0xf8u };
    static const type_unsigned_8 immediate[] = { 0xb8u, 0x34u, 0x12u };
    static const type_unsigned_8 registers[] = { 0x8bu, 0xc1u };

    if (timing_80286_case(nop, sizeof(nop), 3u) ||
        timing_80286_case(clc, sizeof(clc), 2u) ||
        timing_80286_case(immediate, sizeof(immediate), 2u) ||
        timing_80286_case(registers, sizeof(registers), 2u)) return 1;
    if (timing_80286_memory()) return 2;
    if (timing_80286_control_ports()) return 3;
    if (timing_80286_boundaries()) return 4;
    STD_PRINTF("M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK\n");
    return 0;
}
