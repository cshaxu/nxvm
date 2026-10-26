#include "app-mypc110/pc110_pcic_interface.h"
#include "lib/types/test.h"

int main(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80486SX};
    x86_core_machine *machine = LIB_NULL;
    x86_rb5c396 *controller = LIB_NULL;
    lib_u32 value = 0u;
    int result = 0;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_rb5c396_create(&controller) != LIB_STATUS_OK ||
        mypc110_pcic_install_routes(machine, controller) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK) result = 1;
    else if (x86_core_machine_bus_write(machine, X86_RB5C396_INDEX_PORT, 0u) !=
            LIB_STATUS_OK || x86_core_machine_bus_read(machine,
            X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
            value != X86_RB5C396_IDENTIFICATION_VALUE) result = 2;
    else if (x86_core_machine_bus_write(machine, X86_RB5C396_INDEX_PORT, 0x7au) !=
            LIB_STATUS_OK || x86_core_machine_bus_read(machine,
            X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
            value != X86_RB5C396_CHIP_IDENTIFICATION_VALUE) result = 3;
    else if (x86_core_machine_bus_write(machine, X86_RB5C396_DATA_PORT, 0x55u) !=
            LIB_STATUS_OK || x86_core_machine_bus_read(machine,
            X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
            value != X86_RB5C396_CHIP_IDENTIFICATION_VALUE) result = 4;
    else if (x86_core_machine_bus_write(machine, X86_RB5C396_INDEX_PORT, 0x18u) !=
            LIB_STATUS_OK || x86_core_machine_bus_write(machine,
            X86_RB5C396_DATA_PORT, 0x5au) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, X86_RB5C396_DATA_PORT, &value) !=
            LIB_STATUS_OK || value != 0x5au) result = 5;
    x86_rb5c396_destroy(controller);
    x86_core_machine_destroy(machine);
    return result;
}
