#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define T359_S6_RESET_LINEAR 0xfffffff0u
#define T359_S6_RESET_PHYSICAL 0x000ffff0u
#define T359_S6_DATA 0x00001000u

typedef struct t359_s6_state {
    type_unsigned_64 advanced_ticks;
} t359_s6_state;

typedef struct t359_s6_row {
    const type_unsigned_8 *program;
    STD_SIZE_T program_bytes;
    type_unsigned_64 ticks;
    type_unsigned_32 eax;
} t359_s6_row;

static C_VOID t359_s6_reset(C_VOID *opaque)
{
    t359_s6_state *state = (t359_s6_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks = 0u;
}

static C_VOID t359_s6_advance(C_VOID *opaque, type_unsigned_64 ticks)
{
    t359_s6_state *state = (t359_s6_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks += ticks;
}

static const core_machine_execution_provider t359_s6_execution = {
    t359_s6_reset, STD_NULL, t359_s6_advance
};

static C_INT t359_s6_prepare(core_machine **out_machine, t359_s6_state *state)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || state == STD_NULL ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            T359_S6_RESET_LINEAR, T359_S6_RESET_PHYSICAL, 64u) !=
            TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine, T359_S6_DATA,
            T359_S6_DATA, 64u) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &t359_s6_execution, state)) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static C_INT t359_s6_run(core_machine *machine, t359_s6_state *state,
    const t359_s6_row *row)
{
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    type_unsigned_64 data = UINT64_C(0x8877665544332211);

    if (machine == STD_NULL || state == STD_NULL || row == STD_NULL ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, T359_S6_RESET_LINEAR, row->program,
            row->program_bytes) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, T359_S6_DATA, &data, sizeof(data)) !=
            TYPE_STATUS_OK) return 0;
    machine->executor_cpu.data.eax = row->eax;
    return core_machine_run(machine, budget, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 1u &&
        result.ticks == row->ticks && result.elapsed_ticks == row->ticks &&
        state->advanced_ticks == row->ticks;
}

static C_INT t359_s6_test_fixed_real_rows(C_VOID)
{
    static const type_unsigned_8 clts[] = { 0x0fu, 0x06u };
    static const type_unsigned_8 mov_from_cr0[] = { 0x0fu, 0x20u, 0xc0u };
    static const type_unsigned_8 mov_to_cr0[] = { 0x0fu, 0x22u, 0xc0u };
    static const type_unsigned_8 mov_from_tr6[] = { 0x0fu, 0x24u, 0xf0u };
    static const type_unsigned_8 mov_to_tr6[] = { 0x0fu, 0x26u, 0xf0u };
    static const type_unsigned_8 sgdt[] = {
        0x0fu, 0x01u, 0x06u, 0x00u, 0x10u
    };
    static const type_unsigned_8 smsw_register[] = { 0x0fu, 0x01u, 0xe0u };
    static const type_unsigned_8 smsw_memory[] = {
        0x0fu, 0x01u, 0x26u, 0x00u, 0x10u
    };
    static const type_unsigned_8 lmsw_register[] = { 0x0fu, 0x01u, 0xf0u };
    static const type_unsigned_8 push_fs[] = { 0x0fu, 0xa0u };
    static const t359_s6_row rows[] = {
        { clts, sizeof(clts), 5u, 0u },
        { mov_from_cr0, sizeof(mov_from_cr0), 6u, 0u },
        { mov_to_cr0, sizeof(mov_to_cr0), 10u, 0u },
        { mov_from_tr6, sizeof(mov_from_tr6), 12u, 0u },
        { mov_to_tr6, sizeof(mov_to_tr6), 12u, 0u },
        { sgdt, sizeof(sgdt), 9u, 0u },
        { smsw_register, sizeof(smsw_register), 2u, 0u },
        { smsw_memory, sizeof(smsw_memory), 3u, 0u },
        { lmsw_register, sizeof(lmsw_register), 10u, 0u },
        { push_fs, sizeof(push_fs), 2u, 0u }
    };
    t359_s6_state state = { 0u };
    core_machine *machine = STD_NULL;
    STD_SIZE_T index;
    C_INT failed = !t359_s6_prepare(&machine, &state);

    for (index = 0u; !failed && index < sizeof(rows) / sizeof(rows[0]); ++index) {
        failed |= !t359_s6_run(machine, &state, &rows[index]);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s6_test_rejected_lock(C_VOID)
{
    static const type_unsigned_8 locked_clts[] = { 0xf0u, 0x0fu, 0x06u };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    t359_s6_state state = { 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s6_prepare(&machine, &state);

    if (!failed && core_machine_reset(machine) == TYPE_STATUS_OK &&
        test_core_machine_fixture_preflight_real_ud_terminal(machine) &&
        core_machine_memory_write(machine, T359_S6_RESET_LINEAR, locked_clts,
            sizeof(locked_clts)) == TYPE_STATUS_OK) {
        failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_FAULT ||
            result.executed != 0u || result.ticks != 0u ||
            state.advanced_ticks != 0u;
    } else {
        failed = 1;
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    if (t359_s6_test_fixed_real_rows() || t359_s6_test_rejected_lock()) return 1;
    STD_PRINTF("M5:T359:S6:PRIVILEGED-TIMING:OK\n");
    return 0;
}
