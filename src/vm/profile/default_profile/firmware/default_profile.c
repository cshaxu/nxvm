#include "type.h"

#include "vm/profile/default_profile/firmware/default_profile.h"


#include "core/machine/memory.h"

ntvdm64_status vm_profile_default_firmware_compose(
    core_machine_firmware *firmware, vm_profile_default_firmware_plan *out_plan)
{
    static const core_machine_firmware_service_descriptor services[] = {
        { "default_profile.rom", CORE_MACHINE_FIRMWARE_SERVICE_ROM, 10u, 0u },
        { "default_profile.post", CORE_MACHINE_FIRMWARE_SERVICE_POST, 20u, 0u },
        { "bios.int10.video", CORE_MACHINE_FIRMWARE_SERVICE_INTERRUPT, 30u, 0x10u },
        { "bios.int13.disk", CORE_MACHINE_FIRMWARE_SERVICE_INTERRUPT, 40u, 0x13u },
        { "bios.int16.keyboard", CORE_MACHINE_FIRMWARE_SERVICE_INTERRUPT, 50u, 0x16u },
        { "bios.int1a.clock", CORE_MACHINE_FIRMWARE_SERVICE_INTERRUPT, 60u, 0x1au }
    };
    size_t index;
    ntvdm64_status status;

    if (firmware == NULL || out_plan == NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < sizeof(services) / sizeof(services[0]); ++index) {
        status = core_machine_firmware_register_service(firmware, &services[index]);
        if (status != NTVDM64_STATUS_OK) return status;
    }
    out_plan->reset_segment = 0xf000u;
    out_plan->reset_offset = 0xfff0u;
    out_plan->service_count = (C_UINT)(sizeof(services) / sizeof(services[0]));
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_profile_default_firmware_apply_image(
    core_machine *machine, C_INT boot_hdd)
{
    static const uint8_t reset_stub[] = { 0xeau, 0x00u, 0x00u, 0x00u, 0xf0u };
    uint8_t value;
    uint16_t word;
    ntvdm64_status status;

    if (machine == NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    word = 0x03f8u;
    status = core_machine_memory_write(machine, 0x0400u, &word, sizeof(word));
    if (status != NTVDM64_STATUS_OK) return status;
    word = 0x027fu;
    status = core_machine_memory_write(machine, 0x0413u, &word, sizeof(word));
    if (status != NTVDM64_STATUS_OK) return status;
    value = boot_hdd ? 0x80u : 0u;
    status = core_machine_memory_write(machine, 0x0472u, &value, sizeof(value));
    if (status != NTVDM64_STATUS_OK) return status;
    value = 0x55u;
    status = core_machine_memory_write(machine, 0xf0000u, &value, sizeof(value));
    if (status != NTVDM64_STATUS_OK) return status;
    value = 0xaau;
    status = core_machine_memory_write(machine, 0xf0001u, &value, sizeof(value));
    if (status != NTVDM64_STATUS_OK) return status;
    return core_machine_memory_write(machine, 0xffff0u, reset_stub, sizeof(reset_stub));
}

C_VOID vm_profile_default_firmware_cmos_initialize(
    vm_profile_default_firmware_cmos *cmos, C_INT boot_hdd)
{
    if (cmos == NULL) return;
    cmos->equipment = 0x21u;
    cmos->base_memory_kib = 0x7fu;
    cmos->base_memory_kib_high = 0x02u;
    cmos->boot_drive = boot_hdd ? 0x80u : 0u;
}
