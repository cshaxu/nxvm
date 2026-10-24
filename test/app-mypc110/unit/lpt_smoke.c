#include "app-mypc110/pc110_lpt_interface.h"
#include "x86-core/machine/machine_interface.h"

int main(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80486SX};
    x86_core_machine *machine = LIB_NULL;
    mypc110_lpt lpt = {0};
    lib_u32 value = 0u;
    lib_bool failed = LIB_FALSE;

    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        mypc110_lpt_install_routes(machine, &lpt) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK;
    if (!failed) {
        failed |= x86_core_machine_bus_read(machine, 0x03bcu, &value) != LIB_STATUS_OK ||
            value != 0xffu ||
            x86_core_machine_bus_read(machine, 0x03bdu, &value) != LIB_STATUS_OK ||
            value != 0x90u ||
            x86_core_machine_bus_write(machine, 0x03beu, 0x21u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x03bdu, &value) != LIB_STATUS_OK ||
            value != 0xd0u ||
            x86_core_machine_bus_write(machine, 0x03beu, 0xfeu) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x03bdu, &value) != LIB_STATUS_OK ||
            value != 0x90u ||
            x86_core_machine_bus_write(machine, 0x03bcu, 0u) != LIB_STATUS_UNSUPPORTED ||
            x86_core_machine_bus_write(machine, 0x03beu, 0x100u) !=
                LIB_STATUS_INVALID_ARGUMENT;
    }
    x86_core_machine_destroy(machine);
    return failed ? 1 : 0;
}
