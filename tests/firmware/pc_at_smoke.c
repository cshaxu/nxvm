#include <stdio.h>
#include <string.h>

#include "firmware/pc_at.h"

int main(void)
{
    nxvm_firmware firmware;
    nxvm_firmware_pc_at_plan plan;
    nxvm_core_machine *machine = NULL;
    nxvm_core_machine_config config = {
        NXVM_CORE_ABI_VERSION, NXVM_CORE_PROFILE_TEST_MINIMAL, 0u
    };
    uint8_t reset[5];

    nxvm_firmware_initialize(&firmware);
    if (nxvm_firmware_pc_at_compose(&firmware, &plan) != NXVM_CORE_STATUS_OK ||
        plan.reset_segment != 0xf000u || plan.reset_offset != 0xfff0u ||
        plan.service_count != 6u ||
        strcmp(nxvm_firmware_service_at(&firmware, 2u)->id,
               "bios.int10.video") != 0 ||
        nxvm_firmware_freeze(&firmware) != NXVM_CORE_STATUS_OK ||
        nxvm_core_machine_create(&config, &machine) != NXVM_CORE_STATUS_OK ||
        nxvm_core_machine_reset(machine) != NXVM_CORE_STATUS_OK ||
        nxvm_firmware_pc_at_apply_image(machine, 1) != NXVM_CORE_STATUS_OK ||
        nxvm_core_machine_memory_read(machine, 0xffff0u, reset,
                                      sizeof(reset)) != NXVM_CORE_STATUS_OK ||
        reset[0] != 0xeau || reset[4] != 0xf0u) {
        nxvm_core_machine_destroy(machine);
        return 1;
    }
    nxvm_core_machine_destroy(machine);
    puts("M5:T4:S2:PC-AT-FIRMWARE:OK");
    return 0;
}
