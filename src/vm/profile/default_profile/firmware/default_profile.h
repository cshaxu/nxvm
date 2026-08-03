#ifndef NTVDM64_VM_PROFILE_DEFAULT_FIRMWARE_H
#define NTVDM64_VM_PROFILE_DEFAULT_FIRMWARE_H

#include "core/machine/machine_interface.h"
#include "core/machine/firmware_interface.h"

typedef struct vm_profile_default_firmware_plan {
    unsigned reset_segment;
    unsigned reset_offset;
    unsigned service_count;
} vm_profile_default_firmware_plan;

typedef struct vm_profile_default_firmware_cmos {
    uint8_t equipment;
    uint8_t base_memory_kib;
    uint8_t base_memory_kib_high;
    uint8_t boot_drive;
} vm_profile_default_firmware_cmos;

ntvdm64_status vm_profile_default_firmware_compose(
    core_machine_firmware *firmware, vm_profile_default_firmware_plan *out_plan);
ntvdm64_status vm_profile_default_firmware_apply_image(
    core_machine *machine, int boot_hdd);
void vm_profile_default_firmware_cmos_initialize(
    vm_profile_default_firmware_cmos *cmos, int boot_hdd);

#endif
