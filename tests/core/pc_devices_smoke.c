#include <stdio.h>

#include "core/machine/pc_devices.h"

int main(void)
{
    nxvm_core_machine *machine = NULL;
    nxvm_core_machine_config config = { NXVM_CORE_ABI_VERSION, NXVM_CORE_PROFILE_TEST_MINIMAL, 0u };
    nxvm_core_pc_devices devices;
    uint8_t value;
    uint32_t pulses;

    nxvm_core_pc_devices_initialize(&devices);
    if (nxvm_core_machine_create(&config, &machine) != NXVM_CORE_STATUS_OK ||
        nxvm_core_pc_devices_register(machine, &devices) != NXVM_CORE_STATUS_OK ||
        nxvm_core_pc_devices_register(machine, &devices) != NXVM_CORE_STATUS_INVALID_STATE ||
        nxvm_core_pic_raise(&devices, 4u) != NXVM_CORE_STATUS_OK ||
        nxvm_core_pic_raise(&devices, 1u) != NXVM_CORE_STATUS_OK ||
        nxvm_core_pic_acknowledge(&devices, &value) != NXVM_CORE_STATUS_OK || value != 1u ||
        nxvm_core_pic_eoi(&devices, value) != NXVM_CORE_STATUS_OK ||
        nxvm_core_pit_program(&devices, 3u) != NXVM_CORE_STATUS_OK ||
        nxvm_core_pit_tick(&devices, 7u, &pulses) != NXVM_CORE_STATUS_OK || pulses != 2u ||
        nxvm_core_pic_acknowledge(&devices, &value) != NXVM_CORE_STATUS_OK || value != 0u ||
        nxvm_core_dma_request(&devices, 2u) != NXVM_CORE_STATUS_UNSUPPORTED ||
        nxvm_core_dma_register_channel(&devices, 2u) != NXVM_CORE_STATUS_OK ||
        nxvm_core_dma_register_channel(&devices, 2u) != NXVM_CORE_STATUS_INVALID_STATE ||
        nxvm_core_dma_request(&devices, 2u) != NXVM_CORE_STATUS_OK ||
        nxvm_core_dma_acknowledge(&devices, &value) != NXVM_CORE_STATUS_OK || value != 2u) {
        nxvm_core_machine_destroy(machine);
        return 1;
    }
    nxvm_core_pc_devices_reset(&devices);
    nxvm_core_machine_destroy(machine);
    puts("M5:T3:S1:PC-DEVICES:OK");
    return 0;
}
