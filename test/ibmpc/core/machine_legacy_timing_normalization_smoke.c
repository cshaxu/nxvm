#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "x86/core/machine_interface.h"
#include "../board-common/core_machine_board_fixture.h"

#define LEGACY_TIMING_NORMALIZATION_RESET_LINEAR 0xfffffff0u
#define LEGACY_TIMING_NORMALIZATION_RESET_PHYSICAL 0x000ffff0u
#define LEGACY_TIMING_NORMALIZATION_OPERAND_LINEAR 0x00001000u

typedef struct legacy_timing_normalization_state {
    lib_u64 advanced_ticks;
} legacy_timing_normalization_state;

typedef struct legacy_timing_normalization_case {
    core_machine_cpu_profile profile;
    const lib_u8 *program;
    lib_size program_bytes;
    lib_u64 ticks;
    lib_i32 memory;
} legacy_timing_normalization_case;

static void legacy_timing_normalization_reset(void *opaque)
{
    legacy_timing_normalization_state *state = (legacy_timing_normalization_state *)opaque;

    if (state != LIB_NULL) state->advanced_ticks = 0u;
}

static void legacy_timing_normalization_advance(void *opaque, lib_u64 ticks)
{
    legacy_timing_normalization_state *state = (legacy_timing_normalization_state *)opaque;

    if (state != LIB_NULL) state->advanced_ticks += ticks;
}

static const core_machine_execution_provider legacy_timing_normalization_provider = {
    legacy_timing_normalization_reset, legacy_timing_normalization_advance
};

static lib_i32 legacy_timing_normalization_prepare(core_machine_cpu_profile profile,
    core_machine **out_machine, legacy_timing_normalization_state *state)
{
    const core_machine_config config = { .cpu_profile = profile };
    core_machine *machine = LIB_NULL;

    if (out_machine == LIB_NULL || state == LIB_NULL ||
        core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            LEGACY_TIMING_NORMALIZATION_RESET_LINEAR, LEGACY_TIMING_NORMALIZATION_RESET_PHYSICAL, 16u) !=
            LIB_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            LEGACY_TIMING_NORMALIZATION_OPERAND_LINEAR, LEGACY_TIMING_NORMALIZATION_OPERAND_LINEAR, 64u) !=
            LIB_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &legacy_timing_normalization_provider, state)) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 legacy_timing_normalization_run_case(const legacy_timing_normalization_case *test_case)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const lib_u16 operand = 2u;
    core_machine_run_result result = {0};
    legacy_timing_normalization_state state = { 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = test_case == LIB_NULL || !legacy_timing_normalization_prepare(test_case->profile,
        &machine, &state) || core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, LEGACY_TIMING_NORMALIZATION_RESET_LINEAR,
            test_case->program, test_case->program_bytes) != LIB_STATUS_OK;

    if (!failed) {
        test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_EAX, 2u);
        test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_ECX, 2u);
        test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_EDX, 0u);
        failed |= test_case->memory && core_machine_memory_write(machine,
            LEGACY_TIMING_NORMALIZATION_OPERAND_LINEAR, &operand, sizeof(operand)) != LIB_STATUS_OK;
        failed |= test_case->memory && core_machine_memory_write(machine,
            LEGACY_TIMING_NORMALIZATION_OPERAND_LINEAR + 1u, &operand, sizeof(operand)) !=
            LIB_STATUS_OK;
    }
    if (!failed) {
        failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != test_case->ticks ||
            result.elapsed_ticks != test_case->ticks ||
            state.advanced_ticks != test_case->ticks;
    }
    if (failed && test_case != LIB_NULL) {
        lib_c_printf("legacy timing normalization profile=%d expected=%llu actual=%llu executed=%llu reason=%d advanced=%llu opcode=%02x\n",
            (lib_i32)test_case->profile, (unsigned long long)test_case->ticks,
            (unsigned long long)result.ticks, (unsigned long long)result.executed,
            (lib_i32)result.reason, (unsigned long long)state.advanced_ticks,
            test_case->program[0]);
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 legacy_timing_normalization_test_8086(void)
{
    static const lib_u8 mul_byte_register[] = { 0xf6u, 0xe1u };
    static const lib_u8 mul_word_register[] = { 0xf7u, 0xe1u };
    static const lib_u8 mul_byte_memory[] = {
        0xf6u, 0x26u, 0x00u, 0x10u
    };
    static const lib_u8 mul_word_memory[] = {
        0xf7u, 0x26u, 0x00u, 0x10u
    };
    static const lib_u8 imul_byte_register[] = { 0xf6u, 0xe9u };
    static const lib_u8 imul_word_register[] = { 0xf7u, 0xe9u };
    static const lib_u8 imul_byte_memory[] = {
        0xf6u, 0x2eu, 0x00u, 0x10u
    };
    static const lib_u8 imul_word_memory[] = {
        0xf7u, 0x2eu, 0x00u, 0x10u
    };
    static const legacy_timing_normalization_case cases[] = {
        { CORE_MACHINE_CPU_PROFILE_8086, mul_byte_register,
            sizeof(mul_byte_register), 71u, LIB_FALSE },
        { CORE_MACHINE_CPU_PROFILE_8086, mul_word_register,
            sizeof(mul_word_register), 119u, LIB_FALSE },
        { CORE_MACHINE_CPU_PROFILE_8086, mul_byte_memory,
            sizeof(mul_byte_memory), 83u, LIB_TRUE },
        { CORE_MACHINE_CPU_PROFILE_8086, mul_word_memory,
            sizeof(mul_word_memory), 131u, LIB_TRUE },
        { CORE_MACHINE_CPU_PROFILE_8086, imul_byte_register,
            sizeof(imul_byte_register), 91u, LIB_FALSE },
        { CORE_MACHINE_CPU_PROFILE_8086, imul_word_register,
            sizeof(imul_word_register), 139u, LIB_FALSE },
        { CORE_MACHINE_CPU_PROFILE_8086, imul_byte_memory,
            sizeof(imul_byte_memory), 103u, LIB_TRUE },
        { CORE_MACHINE_CPU_PROFILE_8086, imul_word_memory,
            sizeof(imul_word_memory), 151u, LIB_TRUE }
    };
    lib_size index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        if (legacy_timing_normalization_run_case(&cases[index])) return 1;
    }
    return 0;
}

static lib_i32 legacy_timing_normalization_test_80186(void)
{
    static const lib_u8 mul_byte_register[] = { 0xf6u, 0xe1u };
    static const lib_u8 mul_word_register[] = { 0xf7u, 0xe1u };
    static const lib_u8 mul_byte_memory[] = {
        0xf6u, 0x26u, 0x00u, 0x10u
    };
    static const lib_u8 mul_word_memory[] = {
        0xf7u, 0x26u, 0x00u, 0x10u
    };
    static const lib_u8 imul_byte_register[] = { 0xf6u, 0xe9u };
    static const lib_u8 imul_word_register[] = { 0xf7u, 0xe9u };
    static const lib_u8 div_byte_register[] = { 0xf6u, 0xf1u };
    static const lib_u8 div_word_register[] = { 0xf7u, 0xf1u };
    static const lib_u8 idiv_byte_register[] = { 0xf6u, 0xf9u };
    static const lib_u8 idiv_word_register[] = { 0xf7u, 0xf9u };
    static const lib_u8 imul_immediate8_register[] = {
        0x6bu, 0xc1u, 0x02u
    };
    static const lib_u8 imul_immediate16_register[] = {
        0x69u, 0xc1u, 0x02u, 0x00u
    };
    static const lib_u8 imul_immediate8_memory[] = {
        0x6bu, 0x0eu, 0x00u, 0x10u, 0x02u
    };
    static const lib_u8 imul_immediate16_memory[] = {
        0x69u, 0x0eu, 0x00u, 0x10u, 0x02u, 0x00u
    };
    static const lib_u8 imul_immediate8_memory_odd[] = {
        0x6bu, 0x0eu, 0x01u, 0x10u, 0x02u
    };
    static const lib_u8 imul_immediate8_memory_segment[] = {
        0x26u, 0x6bu, 0x0eu, 0x00u, 0x10u, 0x02u
    };
    static const lib_u8 imul_immediate16_memory_segment[] = {
        0x26u, 0x69u, 0x0eu, 0x00u, 0x10u, 0x02u, 0x00u
    };
    static const legacy_timing_normalization_case cases[] = {
        { CORE_MACHINE_CPU_PROFILE_80186, mul_byte_register,
            sizeof(mul_byte_register), 27u, LIB_FALSE },
        { CORE_MACHINE_CPU_PROFILE_80186, mul_word_register,
            sizeof(mul_word_register), 36u, LIB_FALSE },
        { CORE_MACHINE_CPU_PROFILE_80186, mul_byte_memory,
            sizeof(mul_byte_memory), 33u, LIB_TRUE },
        { CORE_MACHINE_CPU_PROFILE_80186, mul_word_memory,
            sizeof(mul_word_memory), 42u, LIB_TRUE },
        { CORE_MACHINE_CPU_PROFILE_80186, imul_byte_register,
            sizeof(imul_byte_register), 27u, LIB_FALSE },
        { CORE_MACHINE_CPU_PROFILE_80186, imul_word_register,
            sizeof(imul_word_register), 36u, LIB_FALSE },
        { CORE_MACHINE_CPU_PROFILE_80186, div_byte_register,
            sizeof(div_byte_register), 29u, LIB_FALSE },
        { CORE_MACHINE_CPU_PROFILE_80186, div_word_register,
            sizeof(div_word_register), 38u, LIB_FALSE },
        { CORE_MACHINE_CPU_PROFILE_80186, idiv_byte_register,
            sizeof(idiv_byte_register), 48u, LIB_FALSE },
        { CORE_MACHINE_CPU_PROFILE_80186, idiv_word_register,
            sizeof(idiv_word_register), 57u, LIB_FALSE },
        { CORE_MACHINE_CPU_PROFILE_80186, imul_immediate8_register,
            sizeof(imul_immediate8_register), 24u, LIB_FALSE },
        { CORE_MACHINE_CPU_PROFILE_80186, imul_immediate8_memory,
            sizeof(imul_immediate8_memory), 24u, LIB_TRUE },
        { CORE_MACHINE_CPU_PROFILE_80186, imul_immediate16_register,
            sizeof(imul_immediate16_register), 31u, LIB_FALSE },
        { CORE_MACHINE_CPU_PROFILE_80186, imul_immediate16_memory,
            sizeof(imul_immediate16_memory), 31u, LIB_TRUE },
        { CORE_MACHINE_CPU_PROFILE_80186, imul_immediate8_memory_odd,
            sizeof(imul_immediate8_memory_odd), 28u, LIB_TRUE },
        { CORE_MACHINE_CPU_PROFILE_80186, imul_immediate8_memory_segment,
            sizeof(imul_immediate8_memory_segment), 26u, LIB_TRUE },
        { CORE_MACHINE_CPU_PROFILE_80186, imul_immediate16_memory_segment,
            sizeof(imul_immediate16_memory_segment), 33u, LIB_TRUE }
    };
    lib_size index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        if (legacy_timing_normalization_run_case(&cases[index])) return 1;
    }
    return 0;
}

static lib_i32 legacy_timing_normalization_test_fault_nonpublication(void)
{
    static const lib_u8 divide_by_zero[] = { 0xf7u, 0xf1u };
    static const lib_u8 handler[] = { 0xf4u };
    static const lib_u16 vector[] = { 0x0100u, 0x0000u };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    legacy_timing_normalization_state state = { 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !legacy_timing_normalization_prepare(CORE_MACHINE_CPU_PROFILE_80186,
        &machine, &state) || core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, LEGACY_TIMING_NORMALIZATION_RESET_LINEAR,
            divide_by_zero, sizeof(divide_by_zero)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0u, vector, sizeof(vector)) !=
            LIB_STATUS_OK || core_machine_memory_write(machine, 0x0100u,
            handler, sizeof(handler)) != LIB_STATUS_OK;

    if (!failed) {
        test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_EAX, 2u);
        test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_EDX, 0u);
        test_core_machine_fixture_write_word(machine, CORE_MACHINE_DEBUG_ECX, 0u);
        {
            lib_status status = core_machine_run(machine, budget, &result);

            failed |= status != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                result.executed != 0u || result.ticks != 0u ||
                result.elapsed_ticks != 0u || state.advanced_ticks != 0u ||
                test_core_machine_fixture_read_register(machine, CORE_MACHINE_DEBUG_EIP) != 0x0100u;
            status = core_machine_run(machine, budget, &result);
            failed |= status != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                result.executed != 1u || result.ticks != 2u ||
                result.elapsed_ticks != 2u || state.advanced_ticks != 2u ||
                test_core_machine_fixture_read_register(machine, CORE_MACHINE_DEBUG_EIP) != 0x0101u;
        }
    }
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    if (legacy_timing_normalization_test_8086()) return 1;
    if (legacy_timing_normalization_test_80186()) return 2;
    if (legacy_timing_normalization_test_fault_nonpublication()) return 3;
    lib_c_printf("LEGACY-TIMING-NORMALIZATION:OK\n");
    return 0;
}