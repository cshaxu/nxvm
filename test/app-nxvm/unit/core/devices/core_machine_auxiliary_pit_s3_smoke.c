#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"

static void program_counter0(core_machine *machine, lib_u16 base,
    lib_u8 control, lib_u16 count)
{
    core_machine_port_write(&machine->executor_port,
        (lib_u16)(base + 3u), control);
    core_machine_port_write(&machine->executor_port, base, count & 0xffu);
    core_machine_port_write(&machine->executor_port, base, count >> 8u);
}

lib_i32 main(void)
{
    core_machine_config config = { 0 };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    config.clock_plan.auxiliary_pit = (core_machine_clock_ratio) {1u, 4u, 0u};
    config.auxiliary_pit_present = LIB_TRUE;
    config.auxiliary_pit_base_port = 0x0048u;
    failed |= core_machine_create(&config, &machine) != LIB_STATUS_OK;
    if (!failed) {
        failed |= !machine->auxiliary_pit_configured ||
            !core_machine_port_has_read(&machine->executor_port, 0x0048u) ||
            !core_machine_port_has_write(&machine->executor_port, 0x004bu) ||
            !core_machine_port_has_read(&machine->executor_port, 0x0040u) ||
            machine->auxiliary_pit.connect.output[0u] != LIB_NULL;
        failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        program_counter0(machine, 0x0040u, 0x30u, 3u);
        program_counter0(machine, 0x0048u, 0x30u, 2u);
        /* A completed count write reaches CE on the following clock. */
        failed |= machine->shared_pit.data.count[0u] != 0u ||
            machine->auxiliary_pit.data.count[0u] != 0u;
        failed |= core_machine_advance_time(machine, 1u) != LIB_STATUS_OK;
        failed |= machine->shared_pit.data.count[0u] != 3u ||
            machine->auxiliary_pit.data.count[0u] != 0u;
        failed |= core_machine_advance_time(machine, 3u) != LIB_STATUS_OK;
        failed |= machine->shared_pit.data.count[0u] != 0u ||
            machine->auxiliary_pit.data.count[0u] != 2u;
        core_machine_port_write(&machine->executor_port, 0x004bu, 0x00u);
        failed |= core_machine_port_read(&machine->executor_port, 0x0048u) != 2u ||
            core_machine_port_read(&machine->executor_port, 0x0048u) != 0u;
        core_machine_port_write(&machine->executor_port, 0x004bu, 0x12u);
        core_machine_port_write(&machine->executor_port, 0x0048u, 0x22u);
        failed |= core_machine_advance_time(machine, 4u) != LIB_STATUS_OK;
        core_machine_port_write(&machine->executor_port, 0x004bu, 0x00u);
        failed |= core_machine_port_read(&machine->executor_port, 0x0048u) != 0x22u ||
            core_machine_port_read(&machine->executor_port, 0x0048u) != 0x22u;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        failed |= machine->shared_pit.data.count[0u] != 0u ||
            machine->auxiliary_pit.data.count[0u] != 0u ||
            !machine->shared_pit.data.flagReady[0u] ||
            !machine->auxiliary_pit.data.flagReady[0u] ||
            machine->auxiliary_pit.connect.output[0u] != LIB_NULL;
    }
    core_machine_destroy(machine);
    if (failed) return 1;
    printf("M5:T386:S3:SECOND-PIT-OWNER:OK\n");
    printf("M5:T386:S3:SECOND-PIT-ISOLATION:OK\n");
    return 0;
}
