#include "type.h"

#include "core/machine/machine.h"

static C_VOID program_counter0(core_machine *machine, type_unsigned_16 base,
    type_unsigned_8 control, type_unsigned_16 count)
{
    core_machine_port_write(&machine->executor_port,
        (type_unsigned_16)(base + 3u), control);
    core_machine_port_write(&machine->executor_port, base, count & 0xffu);
    core_machine_port_write(&machine->executor_port, base, count >> 8u);
}

C_INT main(C_VOID)
{
    core_machine_config config = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    config.auxiliary_pit_present = TYPE_TRUE;
    config.auxiliary_pit_base_port = 0x0048u;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    if (!failed) {
        failed |= !machine->auxiliary_pit_configured ||
            !core_machine_port_has_read(&machine->executor_port, 0x0048u) ||
            !core_machine_port_has_write(&machine->executor_port, 0x004bu) ||
            !core_machine_port_has_read(&machine->executor_port, 0x0040u) ||
            machine->auxiliary_pit.connect.output[0u] != STD_NULL;
        failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
        failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
        program_counter0(machine, 0x0040u, 0x30u, 3u);
        program_counter0(machine, 0x0048u, 0x30u, 2u);
        /* A completed count write reaches CE on the following clock. */
        failed |= machine->shared_pit.data.count[0u] != 0u ||
            machine->auxiliary_pit.data.count[0u] != 0u;
        failed |= core_machine_advance_time(machine, 1u) != TYPE_STATUS_OK;
        failed |= machine->shared_pit.data.count[0u] != 3u ||
            machine->auxiliary_pit.data.count[0u] != 2u;
        failed |= core_machine_advance_time(machine, 1u) != TYPE_STATUS_OK;
        failed |= machine->shared_pit.data.count[0u] != 2u ||
            machine->auxiliary_pit.data.count[0u] != 1u;
        core_machine_port_write(&machine->executor_port, 0x004bu, 0x00u);
        failed |= core_machine_port_read(&machine->executor_port, 0x0048u) != 1u ||
            core_machine_port_read(&machine->executor_port, 0x0048u) != 0u;
        failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
        failed |= machine->shared_pit.data.count[0u] != 0u ||
            machine->auxiliary_pit.data.count[0u] != 0u ||
            !machine->shared_pit.data.flagReady[0u] ||
            !machine->auxiliary_pit.data.flagReady[0u] ||
            machine->auxiliary_pit.connect.output[0u] != STD_NULL;
    }
    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T386:S3:SECOND-PIT-OWNER:OK\n");
    STD_PRINTF("M5:T386:S3:SECOND-PIT-ISOLATION:OK\n");
    return 0;
}
