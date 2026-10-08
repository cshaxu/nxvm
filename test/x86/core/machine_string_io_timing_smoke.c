#include "lib/types/types_interface.h"
#include "x86/core/device_support_interface.h"
#include "lib/types/file.h"

#include "x86/core/machine_interface.h"
#include "debug_fixture.h"
#include "port_mode_fixture.h"

#define STRING_IO_TIMING_RESET_LINEAR 0xfffffff0u
#define STRING_IO_TIMING_RESET_PHYSICAL 0x000ffff0u
#define STRING_IO_TIMING_PORT 0x00e0u

typedef struct string_io_timing_state {
    lib_u64 advanced_ticks;
    lib_u64 setup_ticks;
    lib_u32 reads;
    lib_u32 writes;
    lib_u32 input;
    lib_u32 output;
    lib_u8 fail_reads;
} string_io_timing_state;

typedef struct string_io_timing_string_row {
    lib_u8 opcode;
    lib_u64 ticks[4];
} string_io_timing_string_row;

typedef struct string_io_timing_repeat_row {
    lib_u8 prefix;
    lib_u8 opcode;
    lib_u64 setup[4];
    lib_u64 iteration[4];
} string_io_timing_repeat_row;

static lib_status string_io_timing_port_read(void *owner, lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    string_io_timing_state *state = (string_io_timing_state *)owner;

    if (state == LIB_NULL || out_value == LIB_NULL || port != STRING_IO_TIMING_PORT) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (state->fail_reads) return LIB_STATUS_INTERNAL_ERROR;
    ++state->reads;
    *out_value = state->input;
    return LIB_STATUS_OK;
}

static lib_status string_io_timing_port_write(void *owner, lib_u16 port,
    lib_u32 value)
{
    string_io_timing_state *state = (string_io_timing_state *)owner;

    if (state == LIB_NULL || port != STRING_IO_TIMING_PORT) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    ++state->writes;
    state->output = value;
    return LIB_STATUS_OK;
}

static const core_machine_port_provider string_io_timing_ports = {
    string_io_timing_port_read, string_io_timing_port_write
};

static void string_io_timing_reset(void *opaque)
{
    string_io_timing_state *state = (string_io_timing_state *)opaque;

    if (state != LIB_NULL) lib_memory_set(state, 0, sizeof(*state));
}

static void string_io_timing_advance(void *opaque, lib_u64 ticks)
{
    string_io_timing_state *state = (string_io_timing_state *)opaque;

    if (state != LIB_NULL) state->advanced_ticks += ticks;
}

static const core_machine_execution_provider string_io_timing_execution = {
    string_io_timing_reset, string_io_timing_advance
};

static lib_i32 string_io_timing_prepare(core_machine_cpu_profile profile,
    core_machine **out_machine, string_io_timing_state *state)
{
    const core_machine_executor_config config = { .cpu_profile = profile };
    const lib_u32 reset_linear = profile <= CORE_MACHINE_CPU_PROFILE_80186 ?
        0x000ffff0u : profile == CORE_MACHINE_CPU_PROFILE_80286 ?
        0x00fffff0u : STRING_IO_TIMING_RESET_LINEAR;
    const core_machine_memory_alias_config aliases[] = {
        {STRING_IO_TIMING_RESET_LINEAR, STRING_IO_TIMING_RESET_PHYSICAL, 16u},
        {0x00001000u, 0x00001000u, 64u}
    };
    const core_machine_memory_alias_config narrow_reset = {
        reset_linear, STRING_IO_TIMING_RESET_PHYSICAL, 16u
    };
    core_machine *machine = LIB_NULL;

    if (out_machine == LIB_NULL || state == LIB_NULL ||
        core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_install_memory_aliases(machine, aliases,
            sizeof(aliases) / sizeof(aliases[0]), LIB_FALSE) != LIB_STATUS_OK ||
        (reset_linear != STRING_IO_TIMING_RESET_LINEAR &&
            core_machine_install_memory_aliases(machine, &narrow_reset, 1u,
                LIB_FALSE) != LIB_STATUS_OK) ||
        core_machine_install_port_provider(machine, STRING_IO_TIMING_PORT, STRING_IO_TIMING_PORT,
            &string_io_timing_ports, state) != LIB_STATUS_OK ||
        core_machine_bind_execution_provider(machine, &string_io_timing_execution,
            state) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 string_io_timing_load(core_machine *machine,
    const lib_u8 *program, lib_size program_bytes)
{
    static const lib_u8 source[] = { 0x11u, 0x22u, 0x33u };
    static const lib_u8 compare[] = { 0x11u, 0x44u, 0x33u };

    if (machine == LIB_NULL || program == LIB_NULL ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, STRING_IO_TIMING_RESET_LINEAR, program,
            program_bytes) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x00001000u, source,
            sizeof(source)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x00001100u, compare,
            sizeof(compare)) != LIB_STATUS_OK) {
        return 0;
    }
    test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_ESI, 0x1000u);
    test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_EDI, 0x1100u);
    test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_EDX, STRING_IO_TIMING_PORT);
    test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_EAX, 0x0011u);
    return 1;
}

static lib_i32 string_io_timing_enter_mode(core_machine *machine,
    string_io_timing_state *state, lib_i32 mode, const lib_u8 *program, lib_size bytes)
{
    if (!test_core_80386_enter_port_mode(machine, mode, program, bytes, 0u,
            &state->setup_ticks) || state->advanced_ticks != state->setup_ticks)
        return 0;
    test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_ESI, 0x1000u);
    test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_EDI, 0x1100u);
    test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_EDX, STRING_IO_TIMING_PORT);
    test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_EAX, 0x0011u);
    return 1;
}

static lib_i32 string_io_timing_run(core_machine *machine, string_io_timing_state *state,
    lib_u64 instructions, lib_u64 ticks)
{
    const core_machine_run_budget budget = { instructions, 0u };
    core_machine_run_result result;
    lib_status status = core_machine_run(machine, budget, &result);
    return status == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        result.executed == instructions && result.ticks == ticks &&
        result.elapsed_ticks == state->setup_ticks + ticks &&
        state->advanced_ticks == state->setup_ticks + ticks;
}

static lib_i32 string_io_timing_test_primitives(core_machine_cpu_profile profile,
    lib_u32 profile_index)
{
    static const string_io_timing_string_row rows[] = {
        { 0xa4u, { 18u, 14u, 5u, 7u } },
        { 0xa6u, { 22u, 22u, 8u, 9u } },
        { 0xaau, { 11u, 10u, 3u, 4u } },
        { 0xacu, { 12u, 12u, 5u, 5u } },
        { 0xaeu, { 15u, 15u, 7u, 7u } }
    };
    string_io_timing_state state;
    core_machine *machine = LIB_NULL;
    lib_size index;
    lib_i32 failed = !string_io_timing_prepare(profile, &machine, &state);

    for (index = 0u; !failed && index < sizeof(rows) / sizeof(rows[0]); ++index) {
        failed |= !string_io_timing_load(machine, &rows[index].opcode, 1u) ||
            !string_io_timing_run(machine, &state, 1u, rows[index].ticks[profile_index]);
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 string_io_timing_test_port_primitives(core_machine_cpu_profile profile,
    lib_u32 profile_index)
{
    static const string_io_timing_string_row rows[] = {
        { 0x6cu, { 0u, 14u, 5u, 15u } },
        { 0x6eu, { 0u, 14u, 5u, 12u } }
    };
    string_io_timing_state state;
    core_machine *machine = LIB_NULL;
    lib_size index;
    lib_i32 failed = !string_io_timing_prepare(profile, &machine, &state);

    for (index = 0u; !failed && index < sizeof(rows) / sizeof(rows[0]); ++index) {
        failed |= !string_io_timing_load(machine, &rows[index].opcode, 1u);
        if (!failed) state.input = 0x5au;
        if (!failed) failed |= !string_io_timing_run(machine, &state, 1u,
            rows[index].ticks[profile_index]) ||
            (rows[index].opcode == 0x6cu ? state.reads != 1u ||
                state.writes != 0u : state.reads != 0u || state.writes != 1u);
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 string_io_timing_test_rep_movs(core_machine_cpu_profile profile,
    lib_u64 setup, lib_u64 iteration)
{
    static const lib_u8 program[] = { 0xf3u, 0xa4u };
    static const lib_u32 counts[] = { 0u, 1u, 3u };
    string_io_timing_state state;
    core_machine *machine = LIB_NULL;
    lib_size index;
    lib_i32 failed = !string_io_timing_prepare(profile, &machine, &state);

    for (index = 0u; !failed && index < sizeof(counts) / sizeof(counts[0]); ++index) {
        lib_u64 instructions = counts[index] == 0u ? 1u : counts[index];
        lib_u64 ticks = setup + iteration * counts[index];

        failed |= !string_io_timing_load(machine, program, sizeof(program));
        if (!failed) test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_ECX, (lib_u16)counts[index]);
        if (!failed) failed |= !string_io_timing_run(machine, &state, instructions, ticks);
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 string_io_timing_test_rep_cmps_stop(core_machine_cpu_profile profile,
    lib_u64 setup, lib_u64 iteration)
{
    static const lib_u8 program[] = { 0xf3u, 0xa6u };
    string_io_timing_state state;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !string_io_timing_prepare(profile, &machine, &state);

    if (!failed) failed |= !string_io_timing_load(machine, program, sizeof(program)) ||
        (test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_ECX, 3u), 0) ||
        !string_io_timing_run(machine, &state, 2u, setup + 2u * iteration) ||
        (lib_u16)test_core_machine_fixture_read_register(machine, CORE_MACHINE_DEBUG_ECX) != 1u;
    core_machine_destroy(machine);
    return failed;
}

/* These forms all use REP's ordinary count-to-zero path; CMPS keeps its
 * separately-owned early-stop proof above.  Exercising zero, first, and
 * continuation retirement for every form keeps the source formulas coupled
 * to the stateful publisher rather than to one MOVSB representative. */
static lib_i32 string_io_timing_test_rep_basic_strings(core_machine_cpu_profile profile,
    lib_u32 profile_index)
{
    static const string_io_timing_repeat_row rows[] = {
        { 0xf3u, 0xa4u, { 9u, 8u, 5u, 5u }, { 17u, 8u, 4u, 4u } },
        { 0xf3u, 0xaau, { 9u, 6u, 4u, 5u }, { 10u, 9u, 3u, 5u } },
        { 0xf3u, 0xacu, { 9u, 6u, 0u, 5u }, { 13u, 11u, 0u, 6u } },
        { 0xf2u, 0xaeu, { 9u, 5u, 5u, 5u }, { 15u, 15u, 8u, 8u } }
    };
    static const lib_u32 counts[] = { 0u, 1u, 3u };
    string_io_timing_state state;
    core_machine *machine = LIB_NULL;
    lib_size row;
    lib_size count;
    lib_i32 failed = !string_io_timing_prepare(profile, &machine, &state);

    for (row = 0u; !failed && row < sizeof(rows) / sizeof(rows[0]); ++row) {
        /* Appendix B has no selected 80286 REP LODS formula. */
        if (profile == CORE_MACHINE_CPU_PROFILE_80286 &&
            rows[row].opcode == 0xacu) continue;
        for (count = 0u; !failed && count < sizeof(counts) / sizeof(counts[0]);
            ++count) {
            const lib_u8 program[] = { rows[row].prefix,
                rows[row].opcode };
            lib_u64 instructions = counts[count] == 0u ? 1u :
                counts[count];
            lib_u64 ticks = rows[row].setup[profile_index] +
                rows[row].iteration[profile_index] * counts[count];

            failed |= !string_io_timing_load(machine, program, sizeof(program));
            if (!failed) {
                test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_ECX, (lib_u16)counts[count]);
                if (rows[row].opcode == 0xaeu) {
                    test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_EDI, 0x1000u);
                    test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_EAX, 0u);
                }
            }
            if (!failed) failed |= !string_io_timing_run(machine, &state, instructions,
                ticks);
        }
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 string_io_timing_test_rep_port_strings(core_machine_cpu_profile profile,
    lib_u32 profile_index)
{
    static const lib_u8 opcodes[] = { 0x6cu, 0x6eu };
    static const lib_u64 setup[][4] = {
        { 0u, 8u, 5u, 13u }, { 0u, 8u, 5u, 12u }
    };
    static const lib_u64 iteration[][4] = {
        { 0u, 8u, 4u, 6u }, { 0u, 8u, 4u, 5u }
    };
    static const lib_u32 counts[] = { 0u, 1u, 3u };
    string_io_timing_state state;
    core_machine *machine = LIB_NULL;
    lib_size opcode;
    lib_size count;
    lib_i32 failed = !string_io_timing_prepare(profile, &machine, &state);

    for (opcode = 0u; !failed && opcode < sizeof(opcodes) / sizeof(opcodes[0]);
        ++opcode) {
        for (count = 0u; !failed && count < sizeof(counts) / sizeof(counts[0]);
            ++count) {
            const lib_u8 program[] = { 0xf3u, opcodes[opcode] };
            lib_u64 instructions = counts[count] == 0u ? 1u :
                counts[count];
            lib_u64 ticks = setup[opcode][profile_index] +
                iteration[opcode][profile_index] * counts[count];

            failed |= !string_io_timing_load(machine, program, sizeof(program));
            if (!failed) {
                state.input = 0x5au;
                test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_ECX, (lib_u16)counts[count]);
            }
            if (!failed) failed |= !string_io_timing_run(machine, &state, instructions,
                ticks) || (opcodes[opcode] == 0x6cu ?
                    state.reads != counts[count] : state.writes != counts[count]);
        }
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 string_io_timing_test_rep_ins_80386(void)
{
    static const lib_u8 program[] = { 0xf3u, 0x6cu };
    string_io_timing_state state;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !string_io_timing_prepare(CORE_MACHINE_CPU_PROFILE_80386, &machine,
        &state);

    if (!failed) failed |= !string_io_timing_load(machine, program, sizeof(program));
    if (!failed) {
        state.input = 0x5au;
        test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_ECX, 3u);
        failed |= !string_io_timing_run(machine, &state, 3u, 31u) || state.reads != 3u;
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 string_io_timing_test_ordinary_io_80386(void)
{
    static const lib_u8 input[] = { 0x66u, 0xe5u, STRING_IO_TIMING_PORT };
    static const lib_u8 output[] = { 0x67u, 0xeeu };
    string_io_timing_state state;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !string_io_timing_prepare(CORE_MACHINE_CPU_PROFILE_80386, &machine,
        &state);

    if (!failed) failed |= !string_io_timing_load(machine, input, sizeof(input));
    if (!failed) {
        state.input = 0x12345678u;
        failed |= !string_io_timing_run(machine, &state, 1u, 12u) || state.reads != 1u;
    }
    if (!failed) failed |= !string_io_timing_load(machine, output, sizeof(output));
    if (!failed) failed |= !string_io_timing_run(machine, &state, 1u, 11u) ||
        state.writes != 1u;
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 string_io_timing_test_80386_string_port_modes(void)
{
    static const lib_u8 input[] = { 0x6cu };
    static const lib_u8 output[] = { 0x6eu };
    static const lib_u64 input_ticks[] = { 9u, 29u, 29u };
    static const lib_u64 output_ticks[] = { 6u, 26u, 26u };
    lib_i32 mode;

    for (mode = 0; mode != 3; ++mode) {
        string_io_timing_state state;
        core_machine *machine = LIB_NULL;
        lib_i32 failed = !string_io_timing_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &machine, &state) || !string_io_timing_load(machine, input, sizeof(input));

        if (!failed) failed |= !string_io_timing_enter_mode(machine, &state, mode,
            input, sizeof(input));
        if (!failed) {
            state.input = 0x5au;
            failed |= !string_io_timing_run(machine, &state, 1u, input_ticks[mode]) ||
                state.reads != 1u || state.writes != 0u;
        }
        if (!failed) failed |= !string_io_timing_load(machine, output, sizeof(output));
        if (!failed) failed |= !string_io_timing_enter_mode(machine, &state, mode,
            output, sizeof(output));
        if (!failed) failed |= !string_io_timing_run(machine, &state, 1u,
            output_ticks[mode]) || state.reads != 0u || state.writes != 1u;
        core_machine_destroy(machine);
        if (failed) return 1;
    }
    return 0;
}

static lib_i32 string_io_timing_test_80186_preflight(void)
{
    static const lib_u8 program[] = { 0xf3u, 0xa6u };
    const core_machine_run_budget insufficient = { 1u, 26u };
    const core_machine_run_budget sufficient = { 1u, 27u };
    core_machine_run_result result;
    string_io_timing_state state;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !string_io_timing_prepare(CORE_MACHINE_CPU_PROFILE_80186, &machine,
        &state) || !string_io_timing_load(machine, program, sizeof(program));

    if (!failed) test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_ECX, 1u);
    if (!failed) {
        failed |= core_machine_run(machine, insufficient, &result) !=
            LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            result.executed != 0u || result.ticks != 0u ||
            result.elapsed_ticks != 0u || state.advanced_ticks != 0u;
    }
    if (!failed) {
        failed |= core_machine_run(machine, sufficient, &result) !=
            LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            result.executed != 1u || result.ticks != 27u ||
            result.elapsed_ticks != 27u || state.advanced_ticks != 27u;
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 string_io_timing_test_repeat_continuation_reset(void)
{
    static const lib_u8 program[] = { 0xf3u, 0xa4u };
    const core_machine_run_budget one = { 1u, 0u };
    core_machine_run_result result;
    string_io_timing_state state;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !string_io_timing_prepare(CORE_MACHINE_CPU_PROFILE_80386, &machine,
        &state) || !string_io_timing_load(machine, program, sizeof(program));

    if (!failed) test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_ECX, 3u);
    if (!failed) failed |= !string_io_timing_run(machine, &state, 1u, 9u);
    if (!failed) {
        failed |= core_machine_run(machine, one, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != 4u || result.elapsed_ticks != 13u ||
            state.advanced_ticks != 13u;
    }
    if (!failed) failed |= !string_io_timing_load(machine, program, sizeof(program));
    if (!failed) {
        test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_ECX, 3u);
        failed |= !string_io_timing_run(machine, &state, 1u, 9u);
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 string_io_timing_test_80386_attributes_and_failure(void)
{
    static const lib_u8 operand_size[] = { 0x66u, 0xa5u };
    static const lib_u8 address_size[] = { 0x67u, 0xa4u };
    static const lib_u8 repne_cmps[] = { 0xf2u, 0xa6u };
    static const lib_u8 input[] = { 0x6cu };
    const core_machine_run_budget one = { 1u, 0u };
    core_machine_run_result result;
    string_io_timing_state state;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !string_io_timing_prepare(CORE_MACHINE_CPU_PROFILE_80386, &machine,
        &state);

    if (!failed) failed |= !string_io_timing_load(machine, operand_size,
        sizeof(operand_size)) || !string_io_timing_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !string_io_timing_load(machine, address_size,
        sizeof(address_size)) || !string_io_timing_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !string_io_timing_load(machine, repne_cmps,
        sizeof(repne_cmps));
    if (!failed) {
        test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_ECX, 3u);
        failed |= !string_io_timing_run(machine, &state, 1u, 14u) ||
            (lib_u16)test_core_machine_fixture_read_register(machine, CORE_MACHINE_DEBUG_ECX) != 2u;
    }
    if (!failed) failed |= !string_io_timing_load(machine, input, sizeof(input));
    if (!failed) {
        state.fail_reads = LIB_TRUE;
        failed |= core_machine_run(machine, one, &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u ||
            state.advanced_ticks != 0u || state.reads != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    if (string_io_timing_test_primitives(CORE_MACHINE_CPU_PROFILE_8086, 0u) ||
        string_io_timing_test_primitives(CORE_MACHINE_CPU_PROFILE_80186, 1u) ||
        string_io_timing_test_primitives(CORE_MACHINE_CPU_PROFILE_80286, 2u) ||
        string_io_timing_test_primitives(CORE_MACHINE_CPU_PROFILE_80386, 3u)) return 1;
    if (string_io_timing_test_port_primitives(CORE_MACHINE_CPU_PROFILE_80186, 1u) ||
        string_io_timing_test_port_primitives(CORE_MACHINE_CPU_PROFILE_80286, 2u) ||
        string_io_timing_test_port_primitives(CORE_MACHINE_CPU_PROFILE_80386, 3u)) {
        return 2;
    }
    if (string_io_timing_test_rep_movs(CORE_MACHINE_CPU_PROFILE_8086, 9u, 17u) ||
        string_io_timing_test_rep_movs(CORE_MACHINE_CPU_PROFILE_80186, 8u, 8u) ||
        string_io_timing_test_rep_movs(CORE_MACHINE_CPU_PROFILE_80286, 5u, 4u) ||
        string_io_timing_test_rep_movs(CORE_MACHINE_CPU_PROFILE_80386, 5u, 4u)) return 3;
    if (string_io_timing_test_rep_basic_strings(CORE_MACHINE_CPU_PROFILE_8086, 0u) ||
        string_io_timing_test_rep_basic_strings(CORE_MACHINE_CPU_PROFILE_80186, 1u) ||
        string_io_timing_test_rep_basic_strings(CORE_MACHINE_CPU_PROFILE_80286, 2u) ||
        string_io_timing_test_rep_basic_strings(CORE_MACHINE_CPU_PROFILE_80386, 3u) ||
        string_io_timing_test_rep_port_strings(CORE_MACHINE_CPU_PROFILE_80186, 1u) ||
        string_io_timing_test_rep_port_strings(CORE_MACHINE_CPU_PROFILE_80286, 2u) ||
        string_io_timing_test_rep_port_strings(CORE_MACHINE_CPU_PROFILE_80386, 3u)) {
        return 4;
    }
    if (string_io_timing_test_rep_cmps_stop(CORE_MACHINE_CPU_PROFILE_8086, 9u, 22u) ||
        string_io_timing_test_rep_cmps_stop(CORE_MACHINE_CPU_PROFILE_80186, 5u, 22u) ||
        string_io_timing_test_rep_cmps_stop(CORE_MACHINE_CPU_PROFILE_80286, 5u, 9u) ||
        string_io_timing_test_rep_cmps_stop(CORE_MACHINE_CPU_PROFILE_80386, 5u, 9u)) return 5;
    if (string_io_timing_test_rep_ins_80386() || string_io_timing_test_ordinary_io_80386() ||
        string_io_timing_test_80386_string_port_modes() ||
        string_io_timing_test_80186_preflight() ||
        string_io_timing_test_repeat_continuation_reset() ||
        string_io_timing_test_80386_attributes_and_failure()) return 6;
    lib_c_printf("STRING-IO-TIMING:OK\n");
    return 0;
}