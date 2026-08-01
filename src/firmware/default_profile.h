#ifndef NXVM_FIRMWARE_PC_AT_H
#define NXVM_FIRMWARE_PC_AT_H

#include "machine/core/machine.h"
#include "firmware/firmware.h"

typedef struct nxvm_firmware_default_profile_plan {
    unsigned reset_segment;
    unsigned reset_offset;
    unsigned service_count;
} nxvm_firmware_default_profile_plan;

typedef struct nxvm_firmware_default_profile_cmos {
    uint8_t equipment;
    uint8_t base_memory_kib;
    uint8_t base_memory_kib_high;
    uint8_t boot_drive;
} nxvm_firmware_default_profile_cmos;

nxvm_core_status nxvm_firmware_default_profile_compose(
    nxvm_firmware *firmware, nxvm_firmware_default_profile_plan *out_plan);
nxvm_core_status nxvm_firmware_default_profile_apply_image(
    nxvm_core_machine *machine, int boot_hdd);
void nxvm_firmware_default_profile_cmos_initialize(
    nxvm_firmware_default_profile_cmos *cmos, int boot_hdd);

#endif
