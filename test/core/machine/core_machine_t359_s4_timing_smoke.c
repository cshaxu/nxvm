#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define T359_S4_RESET_LINEAR 0xfffffff0u
#define T359_S4_RESET_PHYSICAL 0x000ffff0u
#define T359_S4_PORT 0x00e0u
#define T359_S4_TSS_BASE 0x00000600u
#define T359_S4_IOMAP_BASE 0x0080u

typedef struct t359_s4_state {
    type_unsigned_64 advanced_ticks;
    type_unsigned_32 reads;
    type_unsigned_32 writes;
    type_unsigned_32 input;
    type_unsigned_32 output;
    type_bool fail_reads;
} t359_s4_state;

typedef struct t359_s4_string_row {
    type_unsigned_8 opcode;
    type_unsigned_64 ticks[4];
} t359_s4_string_row;

typedef struct t359_s4_repeat_row {
    type_unsigned_8 prefix;
    type_unsigned_8 opcode;
    type_unsigned_64 setup[4];
    type_unsigned_64 iteration[4];
} t359_s4_repeat_row;

static type_status t359_s4_port_read(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    t359_s4_state *state = (t359_s4_state *)owner;

    if (state == STD_NULL || out_value == STD_NULL || port != T359_S4_PORT) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (state->fail_reads) return TYPE_STATUS_FAULT;
    ++state->reads;
    *out_value = state->input;
    return TYPE_STATUS_OK;
}

static type_status t359_s4_port_write(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 value)
{
    t359_s4_state *state = (t359_s4_state *)owner;

    if (state == STD_NULL || port != T359_S4_PORT) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ++state->writes;
    state->output = value;
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider t359_s4_ports = {
    t359_s4_port_read, t359_s4_port_write
};

static C_VOID t359_s4_reset(C_VOID *opaque)
{
    t359_s4_state *state = (t359_s4_state *)opaque;

    if (state != STD_NULL) STD_MEMSET(state, 0, sizeof(*state));
}

static C_VOID t359_s4_advance(C_VOID *opaque, type_unsigned_64 ticks)
{
    t359_s4_state *state = (t359_s4_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks += ticks;
}

static const core_machine_execution_provider t359_s4_execution = {
    t359_s4_reset, t359_s4_advance
};

static C_INT t359_s4_prepare(core_machine_cpu_profile profile,
    core_machine **out_machine, t359_s4_state *state)
{
    const core_machine_config config = { .cpu_profile = profile };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || state == STD_NULL ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            T359_S4_RESET_LINEAR, T359_S4_RESET_PHYSICAL, 16u) !=
            TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine, 0x00001000u,
            0x00001000u, 64u) != TYPE_STATUS_OK ||
        core_machine_install_port_provider(machine, T359_S4_PORT, T359_S4_PORT,
            &t359_s4_ports, state) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &t359_s4_execution, state)) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static C_INT t359_s4_load(core_machine *machine,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes)
{
    static const type_unsigned_8 source[] = { 0x11u, 0x22u, 0x33u };
    static const type_unsigned_8 compare[] = { 0x11u, 0x44u, 0x33u };

    if (machine == STD_NULL || program == STD_NULL ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, T359_S4_RESET_LINEAR, program,
            program_bytes) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x00001000u, source,
            sizeof(source)) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x00001100u, compare,
            sizeof(compare)) != TYPE_STATUS_OK) {
        return 0;
    }
    machine->executor_cpu.data.si = 0x1000u;
    machine->executor_cpu.data.di = 0x1100u;
    machine->executor_cpu.data.dx = T359_S4_PORT;
    machine->executor_cpu.data.ax = 0x0011u;
    return 1;
}

static C_INT t359_s4_allow_permission(core_machine *machine, C_INT vm86)
{
    const type_unsigned_16 iomap_base = T359_S4_IOMAP_BASE;
    const type_unsigned_8 bitmap = 0u;

    if (machine == STD_NULL) return 0;
    machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
    machine->executor_cpu.data.eflags = vm86 ?
        VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL : 0u;
    machine->executor_cpu.data.cs.selector = 0x001bu;
    machine->executor_cpu.data.cs.dpl = 3u;
    machine->executor_cpu.data.ss.dpl = 3u;
    machine->executor_cpu.data.ds.selector = 0x0023u;
    machine->executor_cpu.data.ds.dpl = 3u;
    machine->executor_cpu.data.es.selector = 0x0023u;
    machine->executor_cpu.data.es.dpl = 3u;
    machine->executor_cpu.data.tr.flagValid = TYPE_TRUE;
    machine->executor_cpu.data.tr.selector = 0x0028u;
    machine->executor_cpu.data.tr.base = T359_S4_TSS_BASE;
    machine->executor_cpu.data.tr.limit = 0x00ffu;
    machine->executor_cpu.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    return core_machine_memory_write(machine, T359_S4_TSS_BASE + 0x66u,
        &iomap_base, sizeof(iomap_base)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, T359_S4_TSS_BASE + iomap_base +
            0x1cu, &bitmap, sizeof(bitmap)) == TYPE_STATUS_OK;
}

static C_INT t359_s4_run(core_machine *machine, t359_s4_state *state,
    type_unsigned_64 instructions, type_unsigned_64 ticks)
{
    const core_machine_run_budget budget = { instructions, 0u };
    core_machine_run_result result;
    type_status status = core_machine_run(machine, budget, &result);
    return status == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        result.executed == instructions && result.ticks == ticks &&
        result.elapsed_ticks == ticks && state->advanced_ticks == ticks;
}

static C_INT t359_s4_test_primitives(core_machine_cpu_profile profile,
    type_unsigned_32 profile_index)
{
    static const t359_s4_string_row rows[] = {
        { 0xa4u, { 18u, 14u, 5u, 7u } },
        { 0xa6u, { 22u, 22u, 8u, 9u } },
        { 0xaau, { 11u, 10u, 3u, 4u } },
        { 0xacu, { 12u, 12u, 5u, 5u } },
        { 0xaeu, { 15u, 15u, 7u, 7u } }
    };
    t359_s4_state state;
    core_machine *machine = STD_NULL;
    STD_SIZE_T index;
    C_INT failed = !t359_s4_prepare(profile, &machine, &state);

    for (index = 0u; !failed && index < sizeof(rows) / sizeof(rows[0]); ++index) {
        failed |= !t359_s4_load(machine, &rows[index].opcode, 1u) ||
            !t359_s4_run(machine, &state, 1u, rows[index].ticks[profile_index]);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s4_test_port_primitives(core_machine_cpu_profile profile,
    type_unsigned_32 profile_index)
{
    static const t359_s4_string_row rows[] = {
        { 0x6cu, { 0u, 14u, 5u, 15u } },
        { 0x6eu, { 0u, 14u, 5u, 12u } }
    };
    t359_s4_state state;
    core_machine *machine = STD_NULL;
    STD_SIZE_T index;
    C_INT failed = !t359_s4_prepare(profile, &machine, &state);

    for (index = 0u; !failed && index < sizeof(rows) / sizeof(rows[0]); ++index) {
        failed |= !t359_s4_load(machine, &rows[index].opcode, 1u);
        if (!failed) state.input = 0x5au;
        if (!failed) failed |= !t359_s4_run(machine, &state, 1u,
            rows[index].ticks[profile_index]) ||
            (rows[index].opcode == 0x6cu ? state.reads != 1u ||
                state.writes != 0u : state.reads != 0u || state.writes != 1u);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s4_test_rep_movs(core_machine_cpu_profile profile,
    type_unsigned_64 setup, type_unsigned_64 iteration)
{
    static const type_unsigned_8 program[] = { 0xf3u, 0xa4u };
    static const type_unsigned_32 counts[] = { 0u, 1u, 3u };
    t359_s4_state state;
    core_machine *machine = STD_NULL;
    STD_SIZE_T index;
    C_INT failed = !t359_s4_prepare(profile, &machine, &state);

    for (index = 0u; !failed && index < sizeof(counts) / sizeof(counts[0]); ++index) {
        type_unsigned_64 instructions = counts[index] == 0u ? 1u : counts[index];
        type_unsigned_64 ticks = setup + iteration * counts[index];

        failed |= !t359_s4_load(machine, program, sizeof(program));
        if (!failed) machine->executor_cpu.data.cx = (type_unsigned_16)counts[index];
        if (!failed) failed |= !t359_s4_run(machine, &state, instructions, ticks);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s4_test_rep_cmps_stop(core_machine_cpu_profile profile,
    type_unsigned_64 setup, type_unsigned_64 iteration)
{
    static const type_unsigned_8 program[] = { 0xf3u, 0xa6u };
    t359_s4_state state;
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s4_prepare(profile, &machine, &state);

    if (!failed) failed |= !t359_s4_load(machine, program, sizeof(program)) ||
        ((machine->executor_cpu.data.cx = 3u), 0) ||
        !t359_s4_run(machine, &state, 2u, setup + 2u * iteration) ||
        machine->executor_cpu.data.cx != 1u;
    core_machine_destroy(machine);
    return failed;
}

/* These forms all use REP's ordinary count-to-zero path; CMPS keeps its
 * separately-owned early-stop proof above.  Exercising zero, first, and
 * continuation retirement for every form keeps the source formulas coupled
 * to the stateful publisher rather than to one MOVSB representative. */
static C_INT t359_s4_test_rep_basic_strings(core_machine_cpu_profile profile,
    type_unsigned_32 profile_index)
{
    static const t359_s4_repeat_row rows[] = {
        { 0xf3u, 0xa4u, { 9u, 8u, 5u, 5u }, { 17u, 8u, 4u, 4u } },
        { 0xf3u, 0xaau, { 9u, 6u, 4u, 5u }, { 10u, 9u, 3u, 5u } },
        { 0xf3u, 0xacu, { 9u, 6u, 0u, 5u }, { 13u, 11u, 0u, 6u } },
        { 0xf2u, 0xaeu, { 9u, 5u, 5u, 5u }, { 15u, 15u, 8u, 8u } }
    };
    static const type_unsigned_32 counts[] = { 0u, 1u, 3u };
    t359_s4_state state;
    core_machine *machine = STD_NULL;
    STD_SIZE_T row;
    STD_SIZE_T count;
    C_INT failed = !t359_s4_prepare(profile, &machine, &state);

    for (row = 0u; !failed && row < sizeof(rows) / sizeof(rows[0]); ++row) {
        /* Appendix B has no selected 80286 REP LODS formula. */
        if (profile == CORE_MACHINE_CPU_PROFILE_80286 &&
            rows[row].opcode == 0xacu) continue;
        for (count = 0u; !failed && count < sizeof(counts) / sizeof(counts[0]);
            ++count) {
            const type_unsigned_8 program[] = { rows[row].prefix,
                rows[row].opcode };
            type_unsigned_64 instructions = counts[count] == 0u ? 1u :
                counts[count];
            type_unsigned_64 ticks = rows[row].setup[profile_index] +
                rows[row].iteration[profile_index] * counts[count];

            failed |= !t359_s4_load(machine, program, sizeof(program));
            if (!failed) {
                machine->executor_cpu.data.cx = (type_unsigned_16)counts[count];
                if (rows[row].opcode == 0xaeu) {
                    machine->executor_cpu.data.di = 0x1000u;
                    machine->executor_cpu.data.ax = 0u;
                }
            }
            if (!failed) failed |= !t359_s4_run(machine, &state, instructions,
                ticks);
        }
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s4_test_rep_port_strings(core_machine_cpu_profile profile,
    type_unsigned_32 profile_index)
{
    static const type_unsigned_8 opcodes[] = { 0x6cu, 0x6eu };
    static const type_unsigned_64 setup[][4] = {
        { 0u, 8u, 5u, 13u }, { 0u, 8u, 5u, 12u }
    };
    static const type_unsigned_64 iteration[][4] = {
        { 0u, 8u, 4u, 6u }, { 0u, 8u, 4u, 5u }
    };
    static const type_unsigned_32 counts[] = { 0u, 1u, 3u };
    t359_s4_state state;
    core_machine *machine = STD_NULL;
    STD_SIZE_T opcode;
    STD_SIZE_T count;
    C_INT failed = !t359_s4_prepare(profile, &machine, &state);

    for (opcode = 0u; !failed && opcode < sizeof(opcodes) / sizeof(opcodes[0]);
        ++opcode) {
        for (count = 0u; !failed && count < sizeof(counts) / sizeof(counts[0]);
            ++count) {
            const type_unsigned_8 program[] = { 0xf3u, opcodes[opcode] };
            type_unsigned_64 instructions = counts[count] == 0u ? 1u :
                counts[count];
            type_unsigned_64 ticks = setup[opcode][profile_index] +
                iteration[opcode][profile_index] * counts[count];

            failed |= !t359_s4_load(machine, program, sizeof(program));
            if (!failed) {
                state.input = 0x5au;
                machine->executor_cpu.data.cx = (type_unsigned_16)counts[count];
            }
            if (!failed) failed |= !t359_s4_run(machine, &state, instructions,
                ticks) || (opcodes[opcode] == 0x6cu ?
                    state.reads != counts[count] : state.writes != counts[count]);
        }
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s4_test_rep_ins_80386(C_VOID)
{
    static const type_unsigned_8 program[] = { 0xf3u, 0x6cu };
    t359_s4_state state;
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s4_prepare(CORE_MACHINE_CPU_PROFILE_80386, &machine,
        &state);

    if (!failed) failed |= !t359_s4_load(machine, program, sizeof(program));
    if (!failed) {
        state.input = 0x5au;
        machine->executor_cpu.data.cx = 3u;
        failed |= !t359_s4_run(machine, &state, 3u, 31u) || state.reads != 3u;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s4_test_ordinary_io_80386(C_VOID)
{
    static const type_unsigned_8 input[] = { 0x66u, 0xe5u, T359_S4_PORT };
    static const type_unsigned_8 output[] = { 0x67u, 0xeeu };
    t359_s4_state state;
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s4_prepare(CORE_MACHINE_CPU_PROFILE_80386, &machine,
        &state);

    if (!failed) failed |= !t359_s4_load(machine, input, sizeof(input));
    if (!failed) {
        state.input = 0x12345678u;
        failed |= !t359_s4_run(machine, &state, 1u, 12u) || state.reads != 1u;
    }
    if (!failed) failed |= !t359_s4_load(machine, output, sizeof(output));
    if (!failed) failed |= !t359_s4_run(machine, &state, 1u, 11u) ||
        state.writes != 1u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s4_test_80386_string_port_modes(C_VOID)
{
    static const type_unsigned_8 input[] = { 0x6cu };
    static const type_unsigned_8 output[] = { 0x6eu };
    static const type_unsigned_64 input_ticks[] = { 9u, 29u, 29u };
    static const type_unsigned_64 output_ticks[] = { 6u, 26u, 26u };
    C_INT mode;

    for (mode = 0; mode != 3; ++mode) {
        t359_s4_state state;
        core_machine *machine = STD_NULL;
        C_INT failed = !t359_s4_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &machine, &state) || !t359_s4_load(machine, input, sizeof(input));

        if (!failed && mode != 0) failed |= !t359_s4_allow_permission(machine,
            mode == 2);
        if (!failed && mode == 0) {
            machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
            machine->executor_cpu.data.cs.selector = 0x0008u;
            machine->executor_cpu.data.cs.dpl = 0u;
            machine->executor_cpu.data.ss.selector = 0x0010u;
            machine->executor_cpu.data.ss.dpl = 0u;
            machine->executor_cpu.data.ds.selector = 0x0010u;
            machine->executor_cpu.data.ds.dpl = 0u;
            machine->executor_cpu.data.es.selector = 0x0010u;
            machine->executor_cpu.data.es.dpl = 0u;
            machine->executor_cpu.data.eflags = 0u;
        }
        if (!failed) {
            state.input = 0x5au;
            failed |= !t359_s4_run(machine, &state, 1u, input_ticks[mode]) ||
                state.reads != 1u || state.writes != 0u;
        }
        if (!failed) failed |= !t359_s4_load(machine, output, sizeof(output));
        if (!failed && mode != 0) failed |= !t359_s4_allow_permission(machine,
            mode == 2);
        if (!failed && mode == 0) {
            machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
            machine->executor_cpu.data.cs.selector = 0x0008u;
            machine->executor_cpu.data.cs.dpl = 0u;
            machine->executor_cpu.data.ss.selector = 0x0010u;
            machine->executor_cpu.data.ss.dpl = 0u;
            machine->executor_cpu.data.ds.selector = 0x0010u;
            machine->executor_cpu.data.ds.dpl = 0u;
            machine->executor_cpu.data.es.selector = 0x0010u;
            machine->executor_cpu.data.es.dpl = 0u;
            machine->executor_cpu.data.eflags = 0u;
        }
        if (!failed) failed |= !t359_s4_run(machine, &state, 1u,
            output_ticks[mode]) || state.reads != 0u || state.writes != 1u;
        core_machine_destroy(machine);
        if (failed) return 1;
    }
    return 0;
}

static C_INT t359_s4_test_80186_preflight(C_VOID)
{
    static const type_unsigned_8 program[] = { 0xf3u, 0xa6u };
    const core_machine_run_budget insufficient = { 1u, 26u };
    const core_machine_run_budget sufficient = { 1u, 27u };
    core_machine_run_result result;
    t359_s4_state state;
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s4_prepare(CORE_MACHINE_CPU_PROFILE_80186, &machine,
        &state) || !t359_s4_load(machine, program, sizeof(program));

    if (!failed) machine->executor_cpu.data.cx = 1u;
    if (!failed) {
        failed |= core_machine_run(machine, insufficient, &result) !=
            TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            result.executed != 0u || result.ticks != 0u ||
            result.elapsed_ticks != 0u || state.advanced_ticks != 0u;
    }
    if (!failed) {
        failed |= core_machine_run(machine, sufficient, &result) !=
            TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            result.executed != 1u || result.ticks != 27u ||
            result.elapsed_ticks != 27u || state.advanced_ticks != 27u;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s4_test_repeat_continuation_reset(C_VOID)
{
    static const type_unsigned_8 program[] = { 0xf3u, 0xa4u };
    const core_machine_run_budget one = { 1u, 0u };
    core_machine_run_result result;
    t359_s4_state state;
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s4_prepare(CORE_MACHINE_CPU_PROFILE_80386, &machine,
        &state) || !t359_s4_load(machine, program, sizeof(program));

    if (!failed) machine->executor_cpu.data.cx = 3u;
    if (!failed) failed |= !t359_s4_run(machine, &state, 1u, 9u);
    if (!failed) {
        failed |= core_machine_run(machine, one, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != 4u || result.elapsed_ticks != 13u ||
            state.advanced_ticks != 13u;
    }
    if (!failed) failed |= !t359_s4_load(machine, program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.cx = 3u;
        failed |= !t359_s4_run(machine, &state, 1u, 9u);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s4_test_80386_attributes_and_failure(C_VOID)
{
    static const type_unsigned_8 operand_size[] = { 0x66u, 0xa5u };
    static const type_unsigned_8 address_size[] = { 0x67u, 0xa4u };
    static const type_unsigned_8 repne_cmps[] = { 0xf2u, 0xa6u };
    static const type_unsigned_8 input[] = { 0x6cu };
    const core_machine_run_budget one = { 1u, 0u };
    core_machine_run_result result;
    t359_s4_state state;
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s4_prepare(CORE_MACHINE_CPU_PROFILE_80386, &machine,
        &state);

    if (!failed) failed |= !t359_s4_load(machine, operand_size,
        sizeof(operand_size)) || !t359_s4_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !t359_s4_load(machine, address_size,
        sizeof(address_size)) || !t359_s4_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !t359_s4_load(machine, repne_cmps,
        sizeof(repne_cmps));
    if (!failed) {
        machine->executor_cpu.data.cx = 3u;
        failed |= !t359_s4_run(machine, &state, 1u, 14u) ||
            machine->executor_cpu.data.cx != 2u;
    }
    if (!failed) failed |= !t359_s4_load(machine, input, sizeof(input));
    if (!failed) {
        state.fail_reads = TYPE_TRUE;
        failed |= core_machine_run(machine, one, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u ||
            state.advanced_ticks != 0u || state.reads != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    if (t359_s4_test_primitives(CORE_MACHINE_CPU_PROFILE_8086, 0u) ||
        t359_s4_test_primitives(CORE_MACHINE_CPU_PROFILE_80186, 1u) ||
        t359_s4_test_primitives(CORE_MACHINE_CPU_PROFILE_80286, 2u) ||
        t359_s4_test_primitives(CORE_MACHINE_CPU_PROFILE_80386, 3u)) return 1;
    if (t359_s4_test_port_primitives(CORE_MACHINE_CPU_PROFILE_80186, 1u) ||
        t359_s4_test_port_primitives(CORE_MACHINE_CPU_PROFILE_80286, 2u) ||
        t359_s4_test_port_primitives(CORE_MACHINE_CPU_PROFILE_80386, 3u)) {
        return 2;
    }
    if (t359_s4_test_rep_movs(CORE_MACHINE_CPU_PROFILE_8086, 9u, 17u) ||
        t359_s4_test_rep_movs(CORE_MACHINE_CPU_PROFILE_80186, 8u, 8u) ||
        t359_s4_test_rep_movs(CORE_MACHINE_CPU_PROFILE_80286, 5u, 4u) ||
        t359_s4_test_rep_movs(CORE_MACHINE_CPU_PROFILE_80386, 5u, 4u)) return 3;
    if (t359_s4_test_rep_basic_strings(CORE_MACHINE_CPU_PROFILE_8086, 0u) ||
        t359_s4_test_rep_basic_strings(CORE_MACHINE_CPU_PROFILE_80186, 1u) ||
        t359_s4_test_rep_basic_strings(CORE_MACHINE_CPU_PROFILE_80286, 2u) ||
        t359_s4_test_rep_basic_strings(CORE_MACHINE_CPU_PROFILE_80386, 3u) ||
        t359_s4_test_rep_port_strings(CORE_MACHINE_CPU_PROFILE_80186, 1u) ||
        t359_s4_test_rep_port_strings(CORE_MACHINE_CPU_PROFILE_80286, 2u) ||
        t359_s4_test_rep_port_strings(CORE_MACHINE_CPU_PROFILE_80386, 3u)) {
        return 4;
    }
    if (t359_s4_test_rep_cmps_stop(CORE_MACHINE_CPU_PROFILE_8086, 9u, 22u) ||
        t359_s4_test_rep_cmps_stop(CORE_MACHINE_CPU_PROFILE_80186, 5u, 22u) ||
        t359_s4_test_rep_cmps_stop(CORE_MACHINE_CPU_PROFILE_80286, 5u, 9u) ||
        t359_s4_test_rep_cmps_stop(CORE_MACHINE_CPU_PROFILE_80386, 5u, 9u)) return 5;
    if (t359_s4_test_rep_ins_80386() || t359_s4_test_ordinary_io_80386() ||
        t359_s4_test_80386_string_port_modes() ||
        t359_s4_test_80186_preflight() ||
        t359_s4_test_repeat_continuation_reset() ||
        t359_s4_test_80386_attributes_and_failure()) return 6;
    STD_PRINTF("M5:T359:S4:STRING-IO-TIMING:OK\n");
    return 0;
}
