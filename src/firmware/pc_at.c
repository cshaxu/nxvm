#include "firmware/pc_at.h"

nxvm_core_status nxvm_firmware_pc_at_compose(
    nxvm_firmware *firmware, nxvm_firmware_pc_at_plan *out_plan)
{
    static const nxvm_firmware_service_descriptor services[] = {
        { "pc_at.rom", NXVM_FIRMWARE_SERVICE_ROM, 10u, 0u },
        { "pc_at.post", NXVM_FIRMWARE_SERVICE_POST, 20u, 0u },
        { "bios.int10.video", NXVM_FIRMWARE_SERVICE_INTERRUPT, 30u, 0x10u },
        { "bios.int13.disk", NXVM_FIRMWARE_SERVICE_INTERRUPT, 40u, 0x13u },
        { "bios.int16.keyboard", NXVM_FIRMWARE_SERVICE_INTERRUPT, 50u, 0x16u },
        { "bios.int1a.clock", NXVM_FIRMWARE_SERVICE_INTERRUPT, 60u, 0x1au }
    };
    size_t index;
    nxvm_core_status status;

    if (firmware == NULL || out_plan == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < sizeof(services) / sizeof(services[0]); ++index) {
        status = nxvm_firmware_register_service(firmware, &services[index]);
        if (status != NXVM_CORE_STATUS_OK) return status;
    }
    out_plan->reset_segment = 0xf000u;
    out_plan->reset_offset = 0xfff0u;
    out_plan->service_count = (unsigned)(sizeof(services) / sizeof(services[0]));
    return NXVM_CORE_STATUS_OK;
}
