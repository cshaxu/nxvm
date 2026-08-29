#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define T359_S2_RESET_LINEAR 0xfffffff0u
#define T359_S2_RESET_PHYSICAL 0x000ffff0u
#define T359_S2_WINDOW_BYTES 16u

typedef struct t359_s2_timing_state {
    type_unsigned_64 advanced_ticks;
} t359_s2_timing_state;

typedef C_INT (*t359_s2_setup)(core_machine *machine, C_VOID *opaque);

typedef struct t359_s2_word_seed {
    type_unsigned_16 ax;
    type_unsigned_16 cx;
    type_unsigned_16 dx;
    type_unsigned_16 memory;
    type_unsigned_32 memory_address;
    C_INT write_memory;
} t359_s2_word_seed;

static C_VOID t359_s2_timing_reset(C_VOID *opaque)
{
    t359_s2_timing_state *state = (t359_s2_timing_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks = 0u;
}

static C_VOID t359_s2_timing_advance(C_VOID *opaque,
    type_unsigned_64 elapsed_ticks)
{
    t359_s2_timing_state *state = (t359_s2_timing_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks += elapsed_ticks;
}

static const core_machine_execution_provider t359_s2_timing_provider = {
    t359_s2_timing_reset,
    t359_s2_timing_advance
};

static C_INT t359_s2_prepare(core_machine_cpu_profile profile,
    core_machine **out_machine, t359_s2_timing_state *state)
{
    const core_machine_config config = { .cpu_profile = profile };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || state == STD_NULL ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            T359_S2_RESET_LINEAR, T359_S2_RESET_PHYSICAL,
            T359_S2_WINDOW_BYTES) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine, 0x00001000u,
            0x00001000u, T359_S2_WINDOW_BYTES) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &t359_s2_timing_provider, state)) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static C_INT t359_s2_run_with_setup(core_machine *machine,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes,
    type_unsigned_64 expected_ticks, t359_s2_timing_state *state,
    t359_s2_setup setup, C_VOID *opaque)
{
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result = { 0 };
    C_INT passed = machine != STD_NULL && program != STD_NULL && state != STD_NULL &&
        core_machine_reset(machine) == TYPE_STATUS_OK &&
        (setup == STD_NULL || setup(machine, opaque)) &&
        core_machine_memory_write(machine, T359_S2_RESET_LINEAR, program,
            program_bytes) == TYPE_STATUS_OK &&
        core_machine_run(machine, budget, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 1u &&
        result.ticks == expected_ticks && result.elapsed_ticks == expected_ticks &&
        state->advanced_ticks == expected_ticks;

    if (!passed) {
        STD_PRINTF("T359 S2 timing expected=%llu actual=%llu executed=%llu reason=%d advanced=%llu opcode=%02x\n",
            (unsigned long long)expected_ticks, (unsigned long long)result.ticks,
            (unsigned long long)result.executed, (C_INT)result.reason,
            state == STD_NULL ? 0ull : (unsigned long long)state->advanced_ticks,
            program == STD_NULL ? 0u : program[0]);
    }
    return passed;
}

static C_INT t359_s2_run(core_machine *machine,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes,
    type_unsigned_64 expected_ticks, t359_s2_timing_state *state)
{
    return t359_s2_run_with_setup(machine, program, program_bytes,
        expected_ticks, state, STD_NULL, STD_NULL);
}

static C_INT t359_s2_seed_words(core_machine *machine, C_VOID *opaque)
{
    const t359_s2_word_seed *seed = (const t359_s2_word_seed *)opaque;

    if (machine == STD_NULL || seed == STD_NULL) return 0;
    machine->executor_cpu.data.ax = seed->ax;
    machine->executor_cpu.data.cx = seed->cx;
    machine->executor_cpu.data.dx = seed->dx;
    return !seed->write_memory || core_machine_memory_write(machine,
        seed->memory_address, &seed->memory, sizeof(seed->memory)) ==
        TYPE_STATUS_OK;
}

static C_INT t359_s2_test_profile_rows(core_machine_cpu_profile profile,
    type_unsigned_64 add_register_ticks, type_unsigned_64 add_memory_ticks,
    type_unsigned_64 sub_read_ticks, type_unsigned_64 mov_immediate_ticks,
    type_unsigned_64 lea_ticks, type_unsigned_64 adjust_ticks,
    type_unsigned_64 conversion_ticks)
{
    static const type_unsigned_8 add_register[] = { 0x01u, 0xc8u };
    static const type_unsigned_8 add_memory[] = { 0x01u, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 sub_read[] = { 0x2bu, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 mov_immediate[] = {
        0xc7u, 0x06u, 0x00u, 0x10u, 0x34u, 0x12u
    };
    static const type_unsigned_8 lea[] = { 0x8du, 0x42u, 0x00u };
    static const type_unsigned_8 aaa[] = { 0x37u };
    static const type_unsigned_8 cwd[] = { 0x99u };
    const type_unsigned_16 memory_value = 1u;
    t359_s2_timing_state state = { 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s2_prepare(profile, &machine, &state);

    if (!failed) {
        failed |= !t359_s2_run(machine, add_register, sizeof(add_register),
            add_register_ticks, &state) || machine->executor_cpu.data.ax != 0u;
    }
    if (!failed) {
        machine->executor_cpu.data.cx = 1u;
        failed |= core_machine_memory_write(machine, 0x1000u, &memory_value,
            sizeof(memory_value)) != TYPE_STATUS_OK || !t359_s2_run(machine,
            add_memory, sizeof(add_memory), add_memory_ticks, &state);
    }
    if (!failed) {
        failed |= core_machine_memory_write(machine, 0x1000u, &memory_value,
            sizeof(memory_value)) != TYPE_STATUS_OK || !t359_s2_run(machine,
            sub_read, sizeof(sub_read), sub_read_ticks, &state);
    }
    if (!failed) {
        failed |= !t359_s2_run(machine, mov_immediate, sizeof(mov_immediate),
            mov_immediate_ticks, &state);
    }
    if (!failed) {
        machine->executor_cpu.data.bp = 0x1000u;
        machine->executor_cpu.data.si = 1u;
        failed |= !t359_s2_run(machine, lea, sizeof(lea), lea_ticks, &state);
    }
    if (!failed) {
        failed |= !t359_s2_run(machine, aaa, sizeof(aaa), adjust_ticks, &state) ||
            !t359_s2_run(machine, cwd, sizeof(cwd), conversion_ticks, &state);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s2_test_setcc(C_VOID)
{
    static const type_unsigned_8 set_register[] = { 0x0fu, 0x95u, 0xc0u };
    static const type_unsigned_8 set_memory[] = {
        0x0fu, 0x95u, 0x06u, 0x00u, 0x10u
    };
    t359_s2_timing_state state = { 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s2_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &machine, &state);

    if (!failed) {
        failed |= !t359_s2_run(machine, set_register, sizeof(set_register), 4u,
            &state) || machine->executor_cpu.data.al != 1u;
    }
    if (!failed) {
        failed |= !t359_s2_run(machine, set_memory, sizeof(set_memory), 5u,
            &state);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s2_test_legacy_odd_word_transfers(C_VOID)
{
    static const type_unsigned_8 add_memory[] = {
        0x01u, 0x0eu, 0x01u, 0x10u
    };
    static const type_unsigned_8 sub_read[] = {
        0x2bu, 0x06u, 0x01u, 0x10u
    };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_64 add_ticks[] = { 30u, 18u, 11u };
    static const type_unsigned_64 read_ticks[] = { 19u, 14u, 9u };
    const t359_s2_word_seed seed = {
        0u, 1u, 0u, 1u, 0x1001u, TYPE_TRUE
    };
    type_unsigned_32 index;

    for (index = 0u; index < sizeof(profiles) / sizeof(profiles[0u]); ++index) {
        t359_s2_timing_state state = { 0u };
        core_machine *machine = STD_NULL;
        C_INT failed = !t359_s2_prepare(profiles[index], &machine, &state);

        if (!failed) {
            failed |= !t359_s2_run_with_setup(machine, add_memory,
                sizeof(add_memory), add_ticks[index], &state,
                t359_s2_seed_words, (C_VOID *)&seed) ||
                !t359_s2_run_with_setup(machine, sub_read,
                    sizeof(sub_read), read_ticks[index], &state,
                    t359_s2_seed_words, (C_VOID *)&seed);
        }
        core_machine_destroy(machine);
        if (failed) return 1;
    }
    return 0;
}

static C_INT t359_s2_test_dynamic_multiply(C_VOID)
{
    static const type_unsigned_8 mul_zero[] = { 0xf7u, 0xe1u };
    static const type_unsigned_8 imul_immediate8[] = { 0x6bu, 0xc1u, 0x04u };
    static const type_unsigned_8 imul_immediate16[] = {
        0x69u, 0xc1u, 0x04u, 0x00u
    };
    const t359_s2_word_seed multiplier = {
        0u, 16u, 0u, 0u, 0u, TYPE_FALSE
    };
    t359_s2_timing_state state = { 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s2_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &machine, &state);

    if (!failed) {
        failed |= !t359_s2_run(machine, mul_zero, sizeof(mul_zero), 9u, &state) ||
            !t359_s2_run(machine, imul_immediate8,
                sizeof(imul_immediate8), 9u, &state) ||
            !t359_s2_run_with_setup(machine, imul_immediate16,
                sizeof(imul_immediate16), 10u, &state, t359_s2_seed_words,
                (C_VOID *)&multiplier);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s2_test_group3_rows(C_VOID)
{
    static const type_unsigned_8 not_register[] = { 0xf7u, 0xd1u };
    static const type_unsigned_8 neg_memory[] = {
        0xf7u, 0x1eu, 0x00u, 0x10u
    };
    static const type_unsigned_8 mul_register[] = { 0xf7u, 0xe1u };
    static const type_unsigned_8 div_register[] = { 0xf7u, 0xf1u };
    static const type_unsigned_8 idiv_register[] = { 0xf7u, 0xf9u };
    static const type_unsigned_8 imul_immediate[] = {
        0x69u, 0xc1u, 0x04u, 0x00u
    };
    static const type_unsigned_8 mul_memory[] = {
        0xf7u, 0x26u, 0x00u, 0x10u
    };
    static const type_unsigned_8 idiv_memory[] = {
        0x66u, 0xf7u, 0x3eu, 0x00u, 0x10u
    };
    const t359_s2_word_seed seed = {
        0u, 1u, 0u, 1u, 0x1000u, TYPE_TRUE
    };
    const core_machine_run_budget insufficient = { 1u, 105u };
    const core_machine_run_budget sufficient = { 1u, 106u };
    t359_s2_timing_state state = { 0u };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s2_prepare(CORE_MACHINE_CPU_PROFILE_8086,
        &machine, &state);

    if (!failed) {
        failed |= !t359_s2_run_with_setup(machine, not_register,
            sizeof(not_register), 3u, &state, t359_s2_seed_words,
            (C_VOID *)&seed) || !t359_s2_run_with_setup(machine, neg_memory,
            sizeof(neg_memory), 22u, &state, t359_s2_seed_words,
            (C_VOID *)&seed);
    }
    core_machine_destroy(machine);
    machine = STD_NULL;
    state.advanced_ticks = 0u;
    failed |= !t359_s2_prepare(CORE_MACHINE_CPU_PROFILE_80186, &machine,
        &state);
    if (!failed) {
        failed |= !t359_s2_run_with_setup(machine, not_register,
            sizeof(not_register), 3u, &state, t359_s2_seed_words,
            (C_VOID *)&seed) || !t359_s2_run_with_setup(machine, neg_memory,
            sizeof(neg_memory), 3u, &state, t359_s2_seed_words,
            (C_VOID *)&seed);
    }
    core_machine_destroy(machine);
    machine = STD_NULL;
    state.advanced_ticks = 0u;
    failed |= !t359_s2_prepare(CORE_MACHINE_CPU_PROFILE_80286, &machine,
        &state);
    if (!failed) {
        failed |= !t359_s2_run_with_setup(machine, not_register,
            sizeof(not_register), 2u, &state, t359_s2_seed_words,
            (C_VOID *)&seed) || !t359_s2_run_with_setup(machine, neg_memory,
            sizeof(neg_memory), 7u, &state, t359_s2_seed_words,
            (C_VOID *)&seed) || !t359_s2_run_with_setup(machine, mul_register,
            sizeof(mul_register), 21u, &state, t359_s2_seed_words,
            (C_VOID *)&seed) || !t359_s2_run_with_setup(machine, div_register,
            sizeof(div_register), 22u, &state, t359_s2_seed_words,
            (C_VOID *)&seed) || !t359_s2_run_with_setup(machine, idiv_register,
            sizeof(idiv_register), 25u, &state, t359_s2_seed_words,
            (C_VOID *)&seed) || !t359_s2_run_with_setup(machine, imul_immediate,
            sizeof(imul_immediate), 21u, &state, t359_s2_seed_words,
            (C_VOID *)&seed);
    }
    core_machine_destroy(machine);
    machine = STD_NULL;
    state.advanced_ticks = 0u;
    failed |= !t359_s2_prepare(CORE_MACHINE_CPU_PROFILE_80386, &machine,
        &state);
    if (!failed) {
        failed |= !t359_s2_run_with_setup(machine, not_register,
            sizeof(not_register), 2u, &state, t359_s2_seed_words,
            (C_VOID *)&seed) || !t359_s2_run_with_setup(machine, neg_memory,
            sizeof(neg_memory), 6u, &state, t359_s2_seed_words,
            (C_VOID *)&seed) || !t359_s2_run_with_setup(machine, div_register,
            sizeof(div_register), 22u, &state, t359_s2_seed_words,
            (C_VOID *)&seed) || !t359_s2_run_with_setup(machine, idiv_register,
            sizeof(idiv_register), 27u, &state, t359_s2_seed_words,
            (C_VOID *)&seed) || !t359_s2_run_with_setup(machine, mul_memory,
            sizeof(mul_memory), 12u, &state, t359_s2_seed_words,
            (C_VOID *)&seed);
    }
    if (!failed) {
        failed |= core_machine_reset(machine) != TYPE_STATUS_OK ||
            !t359_s2_seed_words(machine, (C_VOID *)&seed) ||
            core_machine_memory_write(machine, T359_S2_RESET_LINEAR,
                idiv_memory, sizeof(idiv_memory)) != TYPE_STATUS_OK ||
            core_machine_run(machine, insufficient, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u ||
            state.advanced_ticks != 0u;
    }
    if (!failed) {
        failed |= core_machine_run(machine, sufficient, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != 46u || result.elapsed_ticks != 46u ||
            state.advanced_ticks != 46u;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s2_test_80386_width_prefixes(C_VOID)
{
    static const type_unsigned_8 operand_size_add[] = { 0x66u, 0x01u, 0xc8u };
    static const type_unsigned_8 address_size_add[] = {
        0x67u, 0x01u, 0x0du, 0x00u, 0x10u, 0x00u, 0x00u
    };
    static const type_unsigned_8 combined_mov[] = {
        0x66u, 0x67u, 0xc7u, 0x05u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x34u, 0x12u, 0x00u, 0x00u
    };
    static const type_unsigned_8 combined_setcc[] = {
        0x66u, 0x67u, 0x0fu, 0x95u, 0xc0u
    };
    static const type_unsigned_8 operand_size_mov[] = { 0x66u, 0x89u, 0xc8u };
    static const type_unsigned_8 address_size_mov[] = {
        0x67u, 0x8bu, 0x0du, 0x00u, 0x10u, 0x00u, 0x00u
    };
    static const type_unsigned_8 address_size_moffs[] = {
        0x67u, 0xa1u, 0x00u, 0x10u, 0x00u, 0x00u
    };
    static const type_unsigned_8 operand_size_immediate[] = {
        0x66u, 0xb8u, 0x34u, 0x12u, 0x00u, 0x00u
    };
    static const type_unsigned_8 segment_add[] = {
        0x26u, 0x01u, 0x0eu, 0x00u, 0x10u
    };
    static const type_unsigned_8 locked_add[] = {
        0xf0u, 0x01u, 0x0eu, 0x00u, 0x10u
    };
    t359_s2_timing_state state = { 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s2_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &machine, &state);

    if (!failed) {
        failed |= !t359_s2_run(machine, operand_size_add,
            sizeof(operand_size_add), 2u, &state) ||
            !t359_s2_run(machine, address_size_add, sizeof(address_size_add),
                7u, &state) || !t359_s2_run(machine, combined_mov,
                    sizeof(combined_mov), 2u, &state) ||
            !t359_s2_run(machine, combined_setcc, sizeof(combined_setcc),
                4u, &state) || !t359_s2_run(machine, operand_size_mov,
                    sizeof(operand_size_mov), 2u, &state) ||
            !t359_s2_run(machine, address_size_mov, sizeof(address_size_mov),
                4u, &state) || !t359_s2_run(machine, address_size_moffs,
                    sizeof(address_size_moffs), 4u, &state) ||
            !t359_s2_run(machine, operand_size_immediate,
                sizeof(operand_size_immediate), 2u, &state) ||
            !t359_s2_run(machine, segment_add, sizeof(segment_add), 7u,
                &state) || !t359_s2_run(machine, locked_add,
                    sizeof(locked_add), 7u, &state);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s2_test_legacy_segment_override(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0x26u, 0x01u, 0x0eu, 0x00u, 0x10u
    };
    t359_s2_timing_state state = { 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s2_prepare(CORE_MACHINE_CPU_PROFILE_8086,
        &machine, &state);

    if (!failed) {
        failed |= !t359_s2_run(machine, program, sizeof(program), 24u, &state);
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    if (t359_s2_test_profile_rows(CORE_MACHINE_CPU_PROFILE_8086,
            3u, 22u, 15u, 16u, 13u, 4u, 5u)) return 1;
    if (t359_s2_test_profile_rows(CORE_MACHINE_CPU_PROFILE_80186,
            3u, 10u, 10u, 13u, 6u, 8u, 4u)) return 2;
    if (t359_s2_test_profile_rows(CORE_MACHINE_CPU_PROFILE_80286,
            2u, 7u, 7u, 3u, 4u, 3u, 2u)) return 3;
    if (t359_s2_test_profile_rows(CORE_MACHINE_CPU_PROFILE_80386,
            2u, 7u, 7u, 2u, 2u, 4u, 3u)) return 4;
    if (t359_s2_test_setcc()) return 5;
    if (t359_s2_test_legacy_odd_word_transfers()) return 6;
    if (t359_s2_test_dynamic_multiply()) return 7;
    if (t359_s2_test_group3_rows()) return 8;
    if (t359_s2_test_80386_width_prefixes()) return 9;
    if (t359_s2_test_legacy_segment_override()) return 10;
    STD_PRINTF("M5:T359:S2:ARITHMETIC-DATA-TIMING:OK\n");
    return 0;
}
