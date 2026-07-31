#ifndef NXVM_FIRMWARE_PC_AT_H
#define NXVM_FIRMWARE_PC_AT_H

#include "firmware/firmware.h"

typedef struct nxvm_firmware_pc_at_plan {
    unsigned reset_segment;
    unsigned reset_offset;
    unsigned service_count;
} nxvm_firmware_pc_at_plan;

nxvm_core_status nxvm_firmware_pc_at_compose(
    nxvm_firmware *firmware, nxvm_firmware_pc_at_plan *out_plan);

#endif
