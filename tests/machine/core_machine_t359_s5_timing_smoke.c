#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define T359_S5_RESET_LINEAR 0xfffffff0u
#define T359_S5_RESET_PHYSICAL 0x000ffff0u
#define T359_S5_DATA 0x00001000u

typedef struct t359_s5_state {
    type_unsigned_64 advanced_ticks;
} t359_s5_state;

typedef struct t359_s5_row {
    const type_unsigned_8 *program;
    STD_SIZE_T program_bytes;
    type_unsigned_64 ticks;
    type_unsigned_32 eflags;
    type_unsigned_32 eax;
    type_unsigned_32 ecx;
    type_unsigned_32 esi;
    type_unsigned_32 memory;
} t359_s5_row;

static C_VOID t359_s5_reset(C_VOID *opaque)
{
    t359_s5_state *state = (t359_s5_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks = 0u;
}

static C_VOID t359_s5_advance(C_VOID *opaque, type_unsigned_64 ticks)
{
    t359_s5_state *state = (t359_s5_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks += ticks;
}

static const core_machine_execution_provider t359_s5_execution = {
    t359_s5_reset, t359_s5_advance
};

static C_INT t359_s5_prepare(core_machine **out_machine, t359_s5_state *state)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || state == STD_NULL ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            T359_S5_RESET_LINEAR, T359_S5_RESET_PHYSICAL, 16u) !=
            TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine, T359_S5_DATA,
            T359_S5_DATA, 64u) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine, 0x00002000u,
            0x00002000u, 64u) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &t359_s5_execution, state)) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static C_INT t359_s5_run(core_machine *machine, t359_s5_state *state,
    const t359_s5_row *row)
{
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;

    if (machine == STD_NULL || state == STD_NULL || row == STD_NULL ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, T359_S5_RESET_LINEAR, row->program,
            row->program_bytes) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, T359_S5_DATA, &row->memory,
            sizeof(row->memory)) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x00002000u, &row->memory,
            sizeof(row->memory)) != TYPE_STATUS_OK) return 0;
    machine->executor_cpu.data.eflags = row->eflags;
    machine->executor_cpu.data.eax = row->eax;
    machine->executor_cpu.data.ecx = row->ecx;
    machine->executor_cpu.data.esi = row->esi;
    machine->executor_cpu.data.fs.base = T359_S5_DATA;
    return core_machine_run(machine, budget, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 1u &&
        result.ticks == row->ticks && result.elapsed_ticks == row->ticks &&
        state->advanced_ticks == row->ticks;
}

static C_INT t359_s5_test_secondary_rows(C_VOID)
{
    static const type_unsigned_8 near_not_taken[] = { 0x0fu, 0x85u, 0u, 0u };
    static const type_unsigned_8 near_taken[] = { 0x0fu, 0x84u, 2u, 0u };
    static const type_unsigned_8 bt_register[] = { 0x0fu, 0xa3u, 0xc1u };
    static const type_unsigned_8 bt_memory[] = {
        0x0fu, 0xa3u, 0x0eu, 0u, 0x10u
    };
    static const type_unsigned_8 bts_register[] = { 0x0fu, 0xabu, 0xc1u };
    static const type_unsigned_8 bts_memory[] = {
        0x0fu, 0xabu, 0x0eu, 0u, 0x10u
    };
    static const type_unsigned_8 btc_immediate_memory[] = {
        0x0fu, 0xbau, 0x3eu, 0u, 0x10u, 1u
    };
    static const type_unsigned_8 shld_register[] = {
        0x0fu, 0xa4u, 0xc1u, 1u
    };
    static const type_unsigned_8 shrd_memory[] = {
        0x0fu, 0xadu, 0x0eu, 0u, 0x10u
    };
    static const type_unsigned_8 movzx_register[] = { 0x0fu, 0xb6u, 0xc1u };
    static const type_unsigned_8 movsx_memory[] = {
        0x0fu, 0xbeu, 0x0eu, 0u, 0x10u
    };
    static const type_unsigned_8 movzx_fs_memory[] = {
        0x64u, 0x0fu, 0xb6u, 0x06u, 0u, 0x10u
    };
    static const type_unsigned_8 bsf[] = { 0x0fu, 0xbcu, 0xc1u };
    static const type_unsigned_8 bsr[] = { 0x0fu, 0xbdu, 0xc1u };
    static const type_unsigned_8 imul_register[] = { 0x0fu, 0xafu, 0xc1u };
    static const type_unsigned_8 imul_memory[] = {
        0x0fu, 0xafu, 0x0eu, 0u, 0x10u
    };
    static const t359_s5_row rows[] = {
        { near_not_taken, sizeof(near_not_taken), 3u, VCPU_EFLAGS_ZF, 0u, 0u, 0u, 0u },
        { near_taken, sizeof(near_taken), 9u, VCPU_EFLAGS_ZF, 0u, 0u, 0u, 0u },
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
    t359_s5_state state = { 0u };
    core_machine *machine = STD_NULL;
    STD_SIZE_T index;
    C_INT failed = !t359_s5_prepare(&machine, &state);

    for (index = 0u; !failed && index < sizeof(rows) / sizeof(rows[0]); ++index) {
        if (!t359_s5_run(machine, &state, &rows[index])) {
            STD_FPRINTF(STD_STDERR, "S5 secondary row %u failed\n",
                (type_unsigned_32)index);
            failed = 1;
        }
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s5_test_attributes_and_preflight(C_VOID)
{
    static const type_unsigned_8 operand_size[] = { 0x66u, 0x0fu, 0xb7u, 0xc1u };
    static const type_unsigned_8 address_size[] = { 0x67u, 0x0fu, 0xb6u, 0x06u };
    static const type_unsigned_8 locked_bts[] = {
        0xf0u, 0x0fu, 0xabu, 0x0eu, 0u, 0x10u
    };
    static const type_unsigned_8 illegal_lock[] = { 0xf0u, 0x0fu, 0xa3u, 0xc1u };
    static const type_unsigned_8 bsr_zero[] = { 0x66u, 0x0fu, 0xbdu, 0xc1u };
    const t359_s5_row rows[] = {
        { operand_size, sizeof(operand_size), 3u, 0u, 0u, 0x0080u, 0u, 0u },
        { address_size, sizeof(address_size), 6u, 0u, 0u, 0u, T359_S5_DATA, 0x80u },
        { locked_bts, sizeof(locked_bts), 13u, 0u, 0u, 0u, 0u, 1u },
        { bsr_zero, sizeof(bsr_zero), 105u, 0u, 0u, 0u, 0u, 0u }
    };
    const core_machine_run_budget insufficient = { 1u, 105u };
    const core_machine_run_budget sufficient = { 1u, 106u };
    core_machine_run_result result;
    t359_s5_state state = { 0u };
    core_machine *machine = STD_NULL;
    STD_SIZE_T index;
    C_INT failed = !t359_s5_prepare(&machine, &state);

    for (index = 0u; !failed && index < sizeof(rows) / sizeof(rows[0]); ++index) {
        failed |= !t359_s5_run(machine, &state, &rows[index]);
    }
    if (!failed && core_machine_reset(machine) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, T359_S5_RESET_LINEAR, bsr_zero,
            sizeof(bsr_zero)) == TYPE_STATUS_OK) {
        failed |= core_machine_run(machine, insufficient, &result) != TYPE_STATUS_OK ||
            result.executed != 0u || result.ticks != 0u ||
            core_machine_run(machine, sufficient, &result) != TYPE_STATUS_OK ||
            result.executed != 1u || result.ticks != 105u;
    } else {
        failed = 1;
    }
    if (!failed && core_machine_reset(machine) == TYPE_STATUS_OK &&
        test_core_machine_fixture_preflight_real_ud_terminal(machine) &&
        core_machine_memory_write(machine, T359_S5_RESET_LINEAR, illegal_lock,
            sizeof(illegal_lock)) == TYPE_STATUS_OK) {
        failed |= core_machine_run(machine, (core_machine_run_budget){ 1u, 0u },
            &result) != TYPE_STATUS_FAULT || result.executed != 0u ||
            result.ticks != 0u || state.advanced_ticks != 0u;
    } else {
        failed = 1;
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    if (t359_s5_test_secondary_rows() ||
        t359_s5_test_attributes_and_preflight()) return 1;
    STD_PRINTF("M5:T359:S5:SECONDARY-INTEGER-TIMING:OK\n");
    return 0;
}
