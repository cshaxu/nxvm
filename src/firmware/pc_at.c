#include "firmware/pc_at.h"

#include "core/memory.h"

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

nxvm_core_status nxvm_firmware_pc_at_apply_image(
    nxvm_core_machine *machine, int boot_hdd)
{
    static const uint8_t reset_stub[] = { 0xeau, 0x00u, 0x00u, 0x00u, 0xf0u };
    uint8_t value;
    uint16_t word;
    nxvm_core_status status;

    if (machine == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    word = 0x03f8u;
    status = nxvm_core_machine_memory_write(machine, 0x0400u, &word, sizeof(word));
    if (status != NXVM_CORE_STATUS_OK) return status;
    word = 0x027fu;
    status = nxvm_core_machine_memory_write(machine, 0x0413u, &word, sizeof(word));
    if (status != NXVM_CORE_STATUS_OK) return status;
    value = boot_hdd ? 0x80u : 0u;
    status = nxvm_core_machine_memory_write(machine, 0x0472u, &value, sizeof(value));
    if (status != NXVM_CORE_STATUS_OK) return status;
    value = 0x55u;
    status = nxvm_core_machine_memory_write(machine, 0xf0000u, &value, sizeof(value));
    if (status != NXVM_CORE_STATUS_OK) return status;
    value = 0xaau;
    status = nxvm_core_machine_memory_write(machine, 0xf0001u, &value, sizeof(value));
    if (status != NXVM_CORE_STATUS_OK) return status;
    return nxvm_core_machine_memory_write(machine, 0xffff0u, reset_stub, sizeof(reset_stub));
}

void nxvm_firmware_pc_at_cmos_initialize(
    nxvm_firmware_pc_at_cmos *cmos, int boot_hdd)
{
    if (cmos == NULL) return;
    cmos->equipment = 0x21u;
    cmos->base_memory_kib = 0x7fu;
    cmos->base_memory_kib_high = 0x02u;
    cmos->boot_drive = boot_hdd ? 0x80u : 0u;
}
