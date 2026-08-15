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

static C_INT timing_80286_lahf_sahf(C_VOID)
{
    static const type_unsigned_8 lahf[] = { 0x9fu };
    static const type_unsigned_8 sahf[] = { 0x9eu };
    const type_unsigned_32 transferred = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    const type_unsigned_32 preserved = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF |
        VCPU_EFLAGS_OF;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, lahf, sizeof(lahf)) ||
        ((machine->executor_cpu.data.eax = 0x11223344u),
            (machine->executor_cpu.data.eflags = transferred), 0) ||
        !timing_80286_run(machine, &state, 1u, 2u) ||
        machine->executor_cpu.data.eax != 0x1122d744u ||
        machine->executor_cpu.data.eflags != transferred;
    if (!failed) failed |= !timing_80286_load(machine, sahf, sizeof(sahf)) ||
        ((machine->executor_cpu.data.eax = 0x1122d744u),
            (machine->executor_cpu.data.eflags = preserved), 0) ||
        !timing_80286_run(machine, &state, 1u, 2u) ||
        machine->executor_cpu.data.eax != 0x1122d744u ||
        machine->executor_cpu.data.eflags != (preserved | transferred);
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_sreg_store(C_VOID)
{
    static const type_unsigned_8 store_ds_ax[] = { 0x8cu, 0xd8u };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, store_ds_ax,
        sizeof(store_ds_ax)) || ((machine->executor_cpu.data.eax =
        0xaabb0000u), (machine->executor_cpu.data.ds.selector = 0x1357u), 0) ||
        !timing_80286_run(machine, &state, 1u, 2u) ||
        machine->executor_cpu.data.eax != 0xaabb1357u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_sreg_load(C_VOID)
{
    static const type_unsigned_8 load_es_ax[] = { 0x8eu, 0xc0u };
    static const type_unsigned_8 load_ss_ax[] = { 0x8eu, 0xd0u };
    static const type_unsigned_8 load_ds_ax[] = { 0x8eu, 0xd8u };
    static const type_unsigned_8 load_ds_even[] = { 0x8eu, 0x1eu, 0x00u, 0x10u };
    static const type_unsigned_8 load_ds_odd[] = { 0x8eu, 0x1eu, 0x01u, 0x10u };
    static const type_unsigned_8 load_ds_indexed[] = { 0x8eu, 0x5au, 0x01u };
    const type_unsigned_16 selector = 0x1357u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, load_es_ax,
        sizeof(load_es_ax)) || ((machine->executor_cpu.data.eax =
        0xaabb1357u), 0) || !timing_80286_run(machine, &state, 1u, 2u) ||
        machine->executor_cpu.data.es.selector != selector ||
        machine->executor_cpu.data.es.base != 0x13570u;
    if (!failed) failed |= !timing_80286_load(machine, load_ss_ax,
        sizeof(load_ss_ax)) || ((machine->executor_cpu.data.eax =
        0xaabb1357u), 0) || !timing_80286_run(machine, &state, 1u, 2u) ||
        machine->executor_cpu.data.ss.selector != selector ||
        machine->executor_cpu.data.ss.base != 0x13570u;
    if (!failed) failed |= !timing_80286_load(machine, load_ds_ax,
        sizeof(load_ds_ax)) || ((machine->executor_cpu.data.eax =
        0xaabb1357u), 0) || !timing_80286_run(machine, &state, 1u, 2u) ||
        machine->executor_cpu.data.ds.selector != selector ||
        machine->executor_cpu.data.ds.base != 0x13570u;
    if (!failed) failed |= !timing_80286_load(machine, load_ds_even,
        sizeof(load_ds_even)) || core_machine_memory_write(machine, 0x1000u,
        &selector, sizeof(selector)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 5u) ||
        machine->executor_cpu.data.ds.selector != selector;
    if (!failed) failed |= !timing_80286_load(machine, load_ds_odd,
        sizeof(load_ds_odd)) || core_machine_memory_write(machine, 0x1001u,
        &selector, sizeof(selector)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 7u) ||
        machine->executor_cpu.data.ds.selector != selector;
    if (!failed) failed |= !timing_80286_load(machine, load_ds_indexed,
        sizeof(load_ds_indexed)) || ((machine->executor_cpu.data.bp = 0x1000u),
        (machine->executor_cpu.data.si = 0u), 0) || core_machine_memory_write(
            machine, 0x1001u, &selector, sizeof(selector)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 8u) ||
        machine->executor_cpu.data.ds.selector != selector;
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
    static const type_unsigned_8 xlat[] = { 0xd7u };
    static const type_unsigned_8 sreg_store_even[] = { 0x8cu, 0x1eu, 0x00u, 0x10u };
    static const type_unsigned_8 sreg_store_odd[] = { 0x8cu, 0x1eu, 0x01u, 0x10u };
    static const type_unsigned_8 sreg_store_indexed[] = { 0x8cu, 0x5au, 0x01u };
    const type_unsigned_16 value = 0x5aa5u;
    const type_unsigned_16 sreg_value = 0x1357u;
    type_unsigned_16 sreg_read = 0u;
    const type_unsigned_8 xlat_value = 0xa5u;
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
    if (!failed) failed |= !timing_80286_load(machine, xlat, sizeof(xlat)) ||
        ((machine->executor_cpu.data.bx = 0x1000u),
            (machine->executor_cpu.data.al = 1u), 0) ||
        core_machine_memory_write(machine, 0x1001u, &xlat_value,
            sizeof(xlat_value)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 5u) ||
        machine->executor_cpu.data.al != xlat_value;
    if (!failed) failed |= !timing_80286_load(machine, sreg_store_even,
        sizeof(sreg_store_even)) || ((machine->executor_cpu.data.ds.selector =
        sreg_value), 0) || !timing_80286_run(machine, &state, 1u, 3u) ||
        core_machine_memory_read(machine, 0x1000u, &sreg_read,
            sizeof(sreg_read)) != TYPE_STATUS_OK || sreg_read != sreg_value;
    if (!failed) failed |= !timing_80286_load(machine, sreg_store_odd,
        sizeof(sreg_store_odd)) || ((machine->executor_cpu.data.ds.selector =
        sreg_value), 0) || !timing_80286_run(machine, &state, 1u, 5u) ||
        core_machine_memory_read(machine, 0x1001u, &sreg_read,
            sizeof(sreg_read)) != TYPE_STATUS_OK || sreg_read != sreg_value;
    if (!failed) failed |= !timing_80286_load(machine, sreg_store_indexed,
        sizeof(sreg_store_indexed)) || ((machine->executor_cpu.data.ds.selector =
        sreg_value), (machine->executor_cpu.data.bp = 0x1000u),
        (machine->executor_cpu.data.si = 0u), 0) ||
        !timing_80286_run(machine, &state, 1u, 6u) ||
        core_machine_memory_read(machine, 0x1001u, &sreg_read,
            sizeof(sreg_read)) != TYPE_STATUS_OK || sreg_read != sreg_value;
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
    static const type_unsigned_8 shift_word_memory[] = { 0xd1u, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 shift_indexed_memory[] = { 0xd0u, 0x4au, 0x01u };
    static const type_unsigned_8 shift_cl[] = { 0xd2u, 0xc0u };
    static const type_unsigned_8 shift_count[] = { 0xc1u, 0xc0u, 0x04u };
    static const type_unsigned_8 shift_cl_memory[] = { 0xd2u, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 shift_count_memory[] = { 0xc1u, 0x4au, 0x01u, 0x04u };
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
        sizeof(shift_memory)) || !timing_80286_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !timing_80286_load(machine, shift_word_memory,
        sizeof(shift_word_memory)) || !timing_80286_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !timing_80286_load(machine, shift_indexed_memory,
        sizeof(shift_indexed_memory)) || ((machine->executor_cpu.data.bp = 0x1000u),
        (machine->executor_cpu.data.si = 0u), 0) || core_machine_memory_write(
            machine, 0x1001u, source, 1u) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 8u);
    if (!failed) failed |= !timing_80286_load(machine, shift_cl,
        sizeof(shift_cl)) || ((machine->executor_cpu.data.cx = 4u), 0) ||
        !timing_80286_run(machine, &state, 1u, 9u);
    if (!failed) failed |= !timing_80286_load(machine, shift_count,
        sizeof(shift_count)) || !timing_80286_run(machine, &state, 1u, 9u);
    if (!failed) failed |= !timing_80286_load(machine, shift_cl_memory,
        sizeof(shift_cl_memory)) || ((machine->executor_cpu.data.cx = 4u), 0) ||
        !timing_80286_run(machine, &state, 1u, 12u);
    if (!failed) failed |= !timing_80286_load(machine, shift_count_memory,
        sizeof(shift_count_memory)) || ((machine->executor_cpu.data.bp = 0x1000u),
        (machine->executor_cpu.data.si = 0u), 0) || core_machine_memory_write(
            machine, 0x1001u, source, 1u) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 13u);
    if (!failed) failed |= !timing_80286_load(machine, shift_cl,
        sizeof(shift_cl)) || ((machine->executor_cpu.data.cx = 0x24u), 0) ||
        !timing_80286_run(machine, &state, 1u, 9u);
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
    static const type_unsigned_8 cmc[] = { 0xf5u };
    static const type_unsigned_8 stc[] = { 0xf9u };
    static const type_unsigned_8 cld[] = { 0xfcu };
    static const type_unsigned_8 std[] = { 0xfdu };
    static const type_unsigned_8 cli[] = { 0xfau };
    static const type_unsigned_8 sti[] = { 0xfbu };
    static const type_unsigned_8 sahf[] = { 0x9eu };
    static const type_unsigned_8 lahf[] = { 0x9fu };
    static const type_unsigned_8 immediate[] = { 0xb8u, 0x34u, 0x12u };
    static const type_unsigned_8 registers[] = { 0x8bu, 0xc1u };

    if (timing_80286_case(nop, sizeof(nop), 3u) ||
        timing_80286_case(clc, sizeof(clc), 2u) ||
        timing_80286_case(cmc, sizeof(cmc), 2u) ||
        timing_80286_case(stc, sizeof(stc), 2u) ||
        timing_80286_case(cld, sizeof(cld), 2u) ||
        timing_80286_case(std, sizeof(std), 2u) ||
        timing_80286_case(cli, sizeof(cli), 3u) ||
        timing_80286_case(sti, sizeof(sti), 2u) ||
        timing_80286_case(sahf, sizeof(sahf), 2u) ||
        timing_80286_case(lahf, sizeof(lahf), 2u) ||
        timing_80286_case(immediate, sizeof(immediate), 2u) ||
        timing_80286_case(registers, sizeof(registers), 2u)) return 1;
    if (timing_80286_lahf_sahf()) return 5;
    if (timing_80286_sreg_store()) return 6;
    if (timing_80286_sreg_load()) return 7;
    if (timing_80286_memory()) return 2;
    if (timing_80286_control_ports()) return 3;
    if (timing_80286_boundaries()) return 4;
    STD_PRINTF("M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK\n");
    return 0;
}
