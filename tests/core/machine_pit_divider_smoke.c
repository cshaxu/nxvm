#include "type.h"

#include "core/machine/machine_interface.h"

typedef struct pit_divider_probe {
    C_UINT low_transitions;
    C_UINT high_transitions;
} pit_divider_probe;

static C_VOID pit_divider_output(C_VOID *opaque, type_bool asserted)
{
    pit_divider_probe *probe = (pit_divider_probe *)opaque;

    if (probe == STD_NULL) return;
    if (asserted) ++probe->high_transitions;
    else ++probe->low_transitions;
}

C_INT main(C_VOID)
{
    const uint8_t program[8] = { 0x90u, 0x90u, 0x90u, 0x90u,
        0x90u, 0x90u, 0x90u, 0x90u };
    core_machine_config config = { 0 };
    core_machine_run_budget four_instruction_budget = { 4u, 0u };
    core_machine_run_budget two_instruction_budget = { 2u, 0u };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    t_port *port;
    t_pit *pit;
    pit_divider_probe probe = { 0u, 0u };
    C_INT failed = 0;

    config.ticks_per_instruction = 1u;
    config.clock_plan.pit.numerator = 1u;
    config.clock_plan.pit.denominator = 4u;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    port = core_machine_configuration_port_borrow(machine);
    pit = core_machine_configuration_shared_pit_borrow(machine);
    failed |= port == STD_NULL || pit == STD_NULL;
    if (!failed) {
        failed |= core_machine_memory_register_mapping(
            core_machine_configuration_memory_borrow(machine), 0xfffffff0u,
            0x000ffff0u, sizeof(program)) != TYPE_STATUS_OK;
        failed |= core_machine_freeze_execution_providers(machine) !=
            TYPE_STATUS_OK;
        failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
        core_machine_pit_set_output(pit, 0u, pit_divider_output, &probe);
        core_machine_port_write(port, 0x0043u, 0x34u);
        core_machine_port_write(port, 0x0040u, 0x02u);
        core_machine_port_write(port, 0x0040u, 0x00u);
        failed |= core_machine_memory_write(machine, 0xfffffff0u, program,
            sizeof(program)) != TYPE_STATUS_OK;
        failed |= core_machine_run(machine, four_instruction_budget, &result) !=
            TYPE_STATUS_OK;
        failed |= result.executed != 4u || probe.low_transitions != 0u;
        failed |= core_machine_run(machine, four_instruction_budget, &result) !=
            TYPE_STATUS_OK;
        failed |= result.executed != 4u || probe.low_transitions != 1u;

        /* A cold reset must discard a partial elapsed-to-PIT conversion. */
        failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
        probe.low_transitions = 0u;
        probe.high_transitions = 0u;
        core_machine_pit_set_output(pit, 0u, pit_divider_output, &probe);
        core_machine_port_write(port, 0x0043u, 0x30u);
        core_machine_port_write(port, 0x0040u, 0x01u);
        core_machine_port_write(port, 0x0040u, 0x00u);
        failed |= core_machine_memory_write(machine, 0xfffffff0u, program,
            sizeof(program)) != TYPE_STATUS_OK;
        failed |= core_machine_run(machine, two_instruction_budget, &result) !=
            TYPE_STATUS_OK;
        failed |= result.executed != 2u || probe.high_transitions != 0u;
        failed |= core_machine_run(machine, two_instruction_budget, &result) !=
            TYPE_STATUS_OK;
        failed |= result.executed != 2u || probe.high_transitions != 1u;

        /* The same reset origin must reproduce the original divider period. */
        failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
        probe.low_transitions = 0u;
        probe.high_transitions = 0u;
        core_machine_pit_set_output(pit, 0u, pit_divider_output, &probe);
        core_machine_port_write(port, 0x0043u, 0x34u);
        core_machine_port_write(port, 0x0040u, 0x02u);
        core_machine_port_write(port, 0x0040u, 0x00u);
        failed |= core_machine_memory_write(machine, 0xfffffff0u, program,
            sizeof(program)) != TYPE_STATUS_OK;
        failed |= core_machine_run(machine, four_instruction_budget, &result) !=
            TYPE_STATUS_OK;
        failed |= result.executed != 4u || probe.low_transitions != 0u;
        failed |= core_machine_run(machine, four_instruction_budget, &result) !=
            TYPE_STATUS_OK;
        failed |= result.executed != 4u || probe.low_transitions != 1u;
    }
    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T225:S2:PIT-DIVIDER:OK\n");
    return 0;
}
