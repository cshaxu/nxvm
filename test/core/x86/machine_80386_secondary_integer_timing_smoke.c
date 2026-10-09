#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "core/x86/machine_interface.h"
#include "core/x86/debug_interface.h"
#include "exception_fixture.h"

#define SECONDARY_INTEGER_TIMING_RESET_LINEAR 0xfffffff0u
#define SECONDARY_INTEGER_TIMING_RESET_PHYSICAL 0x000ffff0u
#define SECONDARY_INTEGER_TIMING_DATA 0x00001000u

typedef struct secondary_integer_timing_state {
    lib_u64 advanced_ticks;
} secondary_integer_timing_state;

typedef struct secondary_integer_timing_row {
    const lib_u8 *program;
    lib_size program_bytes;
    lib_u64 ticks;
    lib_u32 eflags;
    lib_u32 eax;
    lib_u32 ecx;
    lib_u32 esi;
    lib_u32 memory;
} secondary_integer_timing_row;

static void secondary_integer_timing_reset(void *opaque)
{
    secondary_integer_timing_state *state = (secondary_integer_timing_state *)opaque;

    if (state != LIB_NULL) state->advanced_ticks = 0u;
}

static void secondary_integer_timing_advance(void *opaque, lib_u64 ticks)
{
    secondary_integer_timing_state *state = (secondary_integer_timing_state *)opaque;

    if (state != LIB_NULL) state->advanced_ticks += ticks;
}

static const core_machine_execution_provider secondary_integer_timing_execution = {
    secondary_integer_timing_reset, secondary_integer_timing_advance
};

static lib_i32 secondary_integer_timing_prepare(core_machine **out_machine, secondary_integer_timing_state *state)
{
    const core_machine_executor_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    const core_machine_memory_alias_config aliases[] = {
        { SECONDARY_INTEGER_TIMING_RESET_LINEAR, SECONDARY_INTEGER_TIMING_RESET_PHYSICAL, 16u },
        { SECONDARY_INTEGER_TIMING_DATA, SECONDARY_INTEGER_TIMING_DATA, 64u },
        { 0x2000u, 0x2000u, 64u }
    };
    core_machine *machine = LIB_NULL;

    if (out_machine == LIB_NULL || state == LIB_NULL ||
        core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_install_memory_aliases(machine, aliases,
            sizeof(aliases) / sizeof(aliases[0]), LIB_FALSE) != LIB_STATUS_OK ||
        test_core_exception_block_vector(machine, 6u, state) != LIB_STATUS_OK ||
        core_machine_bind_execution_provider(machine, &secondary_integer_timing_execution,
            state) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 secondary_integer_timing_run(core_machine *machine, secondary_integer_timing_state *state,
    const secondary_integer_timing_row *row)
{
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_FS)
    };

    if (machine == LIB_NULL || state == LIB_NULL || row == LIB_NULL ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, SECONDARY_INTEGER_TIMING_RESET_LINEAR, row->program,
            row->program_bytes) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, SECONDARY_INTEGER_TIMING_DATA, &row->memory,
            sizeof(row->memory)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x00002000u, &row->memory,
            sizeof(row->memory)) != LIB_STATUS_OK) return 0;
    entry.values[CORE_MACHINE_DEBUG_EFLAGS] = row->eflags;
    entry.values[CORE_MACHINE_DEBUG_EAX] = row->eax;
    entry.values[CORE_MACHINE_DEBUG_ECX] = row->ecx;
    entry.values[CORE_MACHINE_DEBUG_ESI] = row->esi;
    /* A real-mode selector supplies the same 1000h base through the owner. */
    entry.values[CORE_MACHINE_DEBUG_FS] = SECONDARY_INTEGER_TIMING_DATA >> 4u;
    if (core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK)
        return 0;
    return core_machine_run(machine, budget, &result) == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 1u &&
        result.ticks == row->ticks && result.elapsed_ticks == row->ticks &&
        state->advanced_ticks == row->ticks;
}

static lib_i32 secondary_integer_timing_test_secondary_rows(void)
{
    static const lib_u8 near_not_taken[] = { 0x0fu, 0x85u, 0u, 0u };
    static const lib_u8 near_taken[] = { 0x0fu, 0x84u, 2u, 0u };
    static const lib_u8 bt_register[] = { 0x0fu, 0xa3u, 0xc1u };
    static const lib_u8 bt_memory[] = {
        0x0fu, 0xa3u, 0x0eu, 0u, 0x10u
    };
    static const lib_u8 bts_register[] = { 0x0fu, 0xabu, 0xc1u };
    static const lib_u8 bts_memory[] = {
        0x0fu, 0xabu, 0x0eu, 0u, 0x10u
    };
    static const lib_u8 btc_immediate_memory[] = {
        0x0fu, 0xbau, 0x3eu, 0u, 0x10u, 1u
    };
    static const lib_u8 shld_register[] = {
        0x0fu, 0xa4u, 0xc1u, 1u
    };
    static const lib_u8 shrd_memory[] = {
        0x0fu, 0xadu, 0x0eu, 0u, 0x10u
    };
    static const lib_u8 movzx_register[] = { 0x0fu, 0xb6u, 0xc1u };
    static const lib_u8 movsx_memory[] = {
        0x0fu, 0xbeu, 0x0eu, 0u, 0x10u
    };
    static const lib_u8 movzx_fs_memory[] = {
        0x64u, 0x0fu, 0xb6u, 0x06u, 0u, 0x10u
    };
    static const lib_u8 bsf[] = { 0x0fu, 0xbcu, 0xc1u };
    static const lib_u8 bsr[] = { 0x0fu, 0xbdu, 0xc1u };
    static const lib_u8 imul_register[] = { 0x0fu, 0xafu, 0xc1u };
    static const lib_u8 imul_memory[] = {
        0x0fu, 0xafu, 0x0eu, 0u, 0x10u
    };
    static const secondary_integer_timing_row rows[] = {
        { near_not_taken, sizeof(near_not_taken), 3u, CORE_MACHINE_DEBUG_EFLAGS_ZF, 0u, 0u, 0u, 0u },
        { near_taken, sizeof(near_taken), 9u, CORE_MACHINE_DEBUG_EFLAGS_ZF, 0u, 0u, 0u, 0u },
        { bt_register, sizeof(bt_register), 3u, 0u, 1u, 0u, 0u, 0u },
        { bt_memory, sizeof(bt_memory), 12u, 0u, 0u, 0u, 0u, 1u },
        { bts_register, sizeof(bts_register), 6u, 0u, 1u, 0u, 0u, 0u },
        { bts_memory, sizeof(bts_memory), 13u, 0u, 0u, 0u, 0u, 1u },
        { btc_immediate_memory, sizeof(btc_immediate_memory), 8u, 0u, 0u, 0u, 0u, 0u },
        { shld_register, sizeof(shld_register), 3u, 0u, 1u, 2u, 0u, 0u },
        { shrd_memory, sizeof(shrd_memory), 7u, 0u, 0u, 2u, 0u, 1u },
        { movzx_register, sizeof(movzx_register), 3u, 0u, 0u, 0x80u, 0u, 0u },
        { movsx_memory, sizeof(movsx_memory), 6u, 0u, 0u, 0u, 0u, 0x80u },
        { movzx_fs_memory, sizeof(movzx_fs_memory), 6u, 0u, 0u, 0u, 0u, 0x80u },
        { bsf, sizeof(bsf), 20u, 0u, 0u, 8u, 0u, 0u },
        { bsr, sizeof(bsr), 45u, 0u, 0u, 8u, 0u, 0u },
        { imul_register, sizeof(imul_register), 10u, 0u, 2u, 16u, 0u, 0u },
        { imul_memory, sizeof(imul_memory), 13u, 0u, 2u, 0u, 0u, 16u }
    };
    secondary_integer_timing_state state = { 0u };
    core_machine *machine = LIB_NULL;
    lib_size index;
    lib_i32 failed = !secondary_integer_timing_prepare(&machine, &state);

    for (index = 0u; !failed && index < sizeof(rows) / sizeof(rows[0]); ++index) {
        if (!secondary_integer_timing_run(machine, &state, &rows[index])) {
            lib_c_fprintf(lib_c_stderr, "S5 secondary row %u failed\n",
                (lib_u32)index);
            failed = 1;
        }
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 secondary_integer_timing_test_attributes_and_preflight(void)
{
    static const lib_u8 operand_size[] = { 0x66u, 0x0fu, 0xb7u, 0xc1u };
    static const lib_u8 address_size[] = { 0x67u, 0x0fu, 0xb6u, 0x06u };
    static const lib_u8 locked_bts[] = {
        0xf0u, 0x0fu, 0xabu, 0x0eu, 0u, 0x10u
    };
    static const lib_u8 illegal_lock[] = { 0xf0u, 0x0fu, 0xa3u, 0xc1u };
    static const lib_u8 bsr_zero[] = { 0x66u, 0x0fu, 0xbdu, 0xc1u };
    const secondary_integer_timing_row rows[] = {
        { operand_size, sizeof(operand_size), 3u, 0u, 0u, 0x0080u, 0u, 0u },
        { address_size, sizeof(address_size), 6u, 0u, 0u, 0u, SECONDARY_INTEGER_TIMING_DATA, 0x80u },
        { locked_bts, sizeof(locked_bts), 13u, 0u, 0u, 0u, 0u, 1u },
        { bsr_zero, sizeof(bsr_zero), 105u, 0u, 0u, 0u, 0u, 0u }
    };
    const core_machine_run_budget insufficient = { 1u, 105u };
    const core_machine_run_budget sufficient = { 1u, 106u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic = {0};
    secondary_integer_timing_state state = { 0u };
    core_machine *machine = LIB_NULL;
    lib_size index;
    lib_i32 failed = !secondary_integer_timing_prepare(&machine, &state);

    for (index = 0u; !failed && index < sizeof(rows) / sizeof(rows[0]); ++index) {
        failed |= !secondary_integer_timing_run(machine, &state, &rows[index]);
    }
    if (!failed && core_machine_reset(machine) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, SECONDARY_INTEGER_TIMING_RESET_LINEAR, bsr_zero,
            sizeof(bsr_zero)) == LIB_STATUS_OK) {
        failed |= core_machine_run(machine, insufficient, &result) != LIB_STATUS_OK ||
            result.executed != 0u || result.ticks != 0u ||
            core_machine_run(machine, sufficient, &result) != LIB_STATUS_OK ||
            result.executed != 1u || result.ticks != 105u;
    } else {
        failed = 1;
    }
    /* REAL_UD_TERMINAL_IVT_REJECT: the host vector read fails; this
     * internal CE is not a successfully serviced or terminal guest UD. */
    if (!failed && core_machine_reset(machine) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, SECONDARY_INTEGER_TIMING_RESET_LINEAR, illegal_lock,
            sizeof(illegal_lock)) == LIB_STATUS_OK) {
        failed |= core_machine_run(machine, (core_machine_run_budget){ 1u, 0u },
            &result) != LIB_STATUS_INTERNAL_ERROR || result.executed != 0u ||
            result.ticks != 0u || state.advanced_ticks != 0u ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.elapsed_ticks != 0u ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
            !diagnostic.first_fault.valid ||
            diagnostic.first_fault.exception_mask != VCPUINS_EXCEPT_CE ||
            diagnostic.first_fault.exception_code != 6u * 4u;
    } else {
        failed = 1;
    }
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    if (secondary_integer_timing_test_secondary_rows() ||
        secondary_integer_timing_test_attributes_and_preflight()) return 1;
    lib_c_printf("80386:SECONDARY-INTEGER-TIMING:OK\n");
    return 0;
}