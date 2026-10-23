#include "app-mypc110/pc110_config_interface.h"
#include "x86-core/machine/machine_interface.h"

int main(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80486SX
    };
    x86_core_machine *machine = LIB_NULL;
    x86_vl82c420 *chip = LIB_NULL;
    lib_u32 value = 0u;
    lib_bool failed = LIB_FALSE;

    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_vl82c420_create(&chip) != LIB_STATUS_OK ||
        mypc110_config_install_routes(machine, chip) != LIB_STATUS_OK ||
        mypc110_config_install_routes(machine, chip) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK;
    if (!failed) {
        failed |= x86_core_machine_bus_write(machine, 0x0074u, 0x80u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x0076u, 0x5au) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x0074u, 0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0076u, &value) != LIB_STATUS_OK ||
            value != 0x5au ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00edu, 0x29u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0x29u ||
            x86_core_machine_bus_write(machine, 0x0024u, 0xb8u) != LIB_STATUS_UNSUPPORTED;
    }
    if (machine != LIB_NULL && chip != LIB_NULL) {
        failed |= x86_core_machine_remove_port_routes(machine, chip) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0076u, &value) != LIB_STATUS_UNSUPPORTED;
    }
    x86_vl82c420_destroy(chip);
    x86_core_machine_destroy(machine);
    return failed ? 1 : 0;
}
