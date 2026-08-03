#include "type.h"




#include "vm/profile/default_profile/firmware/default_profile.h"

C_INT main(C_VOID)
{
    core_machine_firmware firmware;
    vm_profile_default_firmware_plan plan;
    core_machine *machine = STD_NULL;
    core_machine_config config = {
        CORE_MACHINE_PROFILE_TEST_MINIMAL, 0u
    };
    vm_profile_default_firmware_cmos cmos;
    uint8_t reset[5];

    core_machine_firmware_initialize(&firmware);
    if (vm_profile_default_firmware_compose(&firmware, &plan) != NTVDM64_STATUS_OK ||
        plan.reset_segment != 0xf000u || plan.reset_offset != 0xfff0u ||
        plan.service_count != 6u ||
        STD_STRCMP(core_machine_firmware_service_at(&firmware, 2u)->id,
               "bios.int10.video") != 0 ||
        STD_STRCMP(core_machine_firmware_find_interrupt(&firmware, 0x13u)->id,
               "bios.int13.disk") != 0 ||
        core_machine_firmware_find_interrupt(&firmware, 0x19u) != STD_NULL ||
        core_machine_firmware_freeze(&firmware) != NTVDM64_STATUS_OK ||
        core_machine_create(&config, &machine) != NTVDM64_STATUS_OK ||
        core_machine_reset(machine) != NTVDM64_STATUS_OK ||
        vm_profile_default_firmware_apply_image(machine, 1) != NTVDM64_STATUS_OK ||
        core_machine_memory_read(machine, 0xffff0u, reset,
                                      sizeof(reset)) != NTVDM64_STATUS_OK ||
        reset[0] != 0xeau || reset[4] != 0xf0u) {
        core_machine_destroy(machine);
        return 1;
    }
    vm_profile_default_firmware_cmos_initialize(&cmos, 1);
    if (cmos.equipment != 0x21u || cmos.base_memory_kib != 0x7fu ||
        cmos.base_memory_kib_high != 0x02u || cmos.boot_drive != 0x80u) {
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_destroy(machine);
    puts("M5:T4:S2:PC-AT-FIRMWARE:OK");
    return 0;
}
