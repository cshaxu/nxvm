#include <stdio.h>
#include <string.h>

#include "vm/profile/default_profile/firmware/default_profile.h"

int main(void)
{
    nxvm_firmware firmware;
    nxvm_firmware_default_profile_plan plan;
    core_machine *machine = NULL;
    core_machine_config config = {
        CORE_MACHINE_PROFILE_TEST_MINIMAL, 0u
    };
    nxvm_firmware_default_profile_cmos cmos;
    uint8_t reset[5];

    nxvm_firmware_initialize(&firmware);
    if (nxvm_firmware_default_profile_compose(&firmware, &plan) != NXVM_CORE_STATUS_OK ||
        plan.reset_segment != 0xf000u || plan.reset_offset != 0xfff0u ||
        plan.service_count != 6u ||
        strcmp(nxvm_firmware_service_at(&firmware, 2u)->id,
               "bios.int10.video") != 0 ||
        strcmp(nxvm_firmware_find_interrupt(&firmware, 0x13u)->id,
               "bios.int13.disk") != 0 ||
        nxvm_firmware_find_interrupt(&firmware, 0x19u) != NULL ||
        nxvm_firmware_freeze(&firmware) != NXVM_CORE_STATUS_OK ||
        core_machine_create(&config, &machine) != NXVM_CORE_STATUS_OK ||
        core_machine_reset(machine) != NXVM_CORE_STATUS_OK ||
        nxvm_firmware_default_profile_apply_image(machine, 1) != NXVM_CORE_STATUS_OK ||
        core_machine_memory_read(machine, 0xffff0u, reset,
                                      sizeof(reset)) != NXVM_CORE_STATUS_OK ||
        reset[0] != 0xeau || reset[4] != 0xf0u) {
        core_machine_destroy(machine);
        return 1;
    }
    nxvm_firmware_default_profile_cmos_initialize(&cmos, 1);
    if (cmos.equipment != 0x21u || cmos.base_memory_kib != 0x7fu ||
        cmos.base_memory_kib_high != 0x02u || cmos.boot_drive != 0x80u) {
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_destroy(machine);
    puts("M5:T4:S2:PC-AT-FIRMWARE:OK");
    return 0;
}
