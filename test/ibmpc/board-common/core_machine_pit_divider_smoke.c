#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "ibmpc/board-common/machine_board_state.h"

typedef struct pit_divider_probe {
    lib_u32 low_transitions;
    lib_u32 high_transitions;
} pit_divider_probe;

static void pit_divider_output(void *opaque, lib_u8 asserted)
{
    pit_divider_probe *probe = (pit_divider_probe *)opaque;

    if (probe == LIB_NULL) return;
    if (asserted) ++probe->high_transitions;
    else ++probe->low_transitions;
}

static void program_pit_divider(core_machine *machine,
    core_machine_board_state *board, lib_u8 control, lib_u16 divisor,
    pit_divider_probe *probe)
{
    x86_pit_set_output(board->shared_pit, 0u, pit_divider_output, probe);
    if (core_machine_bus_write(machine, 0x43u, control) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x40u, divisor & 0xffu) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x40u, divisor >> 8u) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
}

lib_i32 main(void)
{
    const lib_u8 program[8] = { 0x90u, 0x90u, 0x90u, 0x90u,
        0x90u, 0x90u, 0x90u, 0x90u };
    core_machine_config config = { 0 };
    core_machine_run_budget four_instruction_budget = { 4u, 0u };
    core_machine_run_budget two_instruction_budget = { 2u, 0u };
    core_machine_run_result result;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    pit_divider_probe probe = { 0u, 0u };
    lib_i32 failed = 0;

    config.ticks_per_instruction = 1u;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    config.clock_plan.pit.numerator = 1u;
    config.clock_plan.pit.denominator = 4u;
    failed |= core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;
    if (!failed) {
        const core_machine_memory_alias_config alias = {0x00fffff0u, 0x000ffff0u, 15u};
        failed |= core_machine_install_memory_aliases(machine, &alias, 1u,
            LIB_FALSE) != LIB_STATUS_OK;
        failed |= core_machine_freeze_execution_providers(machine) !=
            LIB_STATUS_OK;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        program_pit_divider(machine, board, 0x34u, 2u, &probe);
        failed |= core_machine_memory_write(machine, 0x00fffff0u, program,
            sizeof(program)) != LIB_STATUS_OK;
        failed |= core_machine_run(machine, four_instruction_budget, &result) !=
            LIB_STATUS_OK;
        failed |= result.executed != 4u || probe.low_transitions != 1u ||
            probe.high_transitions != 0u;
        failed |= core_machine_run(machine, four_instruction_budget, &result) !=
            LIB_STATUS_OK;
        failed |= result.executed != 4u || probe.low_transitions != 2u ||
            probe.high_transitions != 2u;

        /* A cold reset must discard a partial elapsed-to-PIT conversion. */
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        probe.low_transitions = 0u;
        probe.high_transitions = 0u;
        program_pit_divider(machine, board, 0x30u, 1u, &probe);
        failed |= core_machine_memory_write(machine, 0x00fffff0u, program,
            sizeof(program)) != LIB_STATUS_OK;
        failed |= core_machine_run(machine, two_instruction_budget, &result) !=
            LIB_STATUS_OK;
        failed |= result.executed != 2u || probe.high_transitions != 0u;
        failed |= core_machine_run(machine, two_instruction_budget, &result) !=
            LIB_STATUS_OK;
        failed |= result.executed != 2u || probe.high_transitions != 1u;

        /* The same reset origin must reproduce the original divider period. */
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        probe.low_transitions = 0u;
        probe.high_transitions = 0u;
        program_pit_divider(machine, board, 0x34u, 2u, &probe);
        failed |= core_machine_memory_write(machine, 0x00fffff0u, program,
            sizeof(program)) != LIB_STATUS_OK;
        failed |= core_machine_run(machine, four_instruction_budget, &result) !=
            LIB_STATUS_OK;
        failed |= result.executed != 4u || probe.low_transitions != 1u ||
            probe.high_transitions != 0u;
        failed |= core_machine_run(machine, four_instruction_budget, &result) !=
            LIB_STATUS_OK;
        failed |= result.executed != 4u || probe.low_transitions != 2u ||
            probe.high_transitions != 2u;
    }
    core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("PIT-DIVIDER:OK\n");
    return 0;
}
