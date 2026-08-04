/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "vm/profile/default_profile/firmware/context.h"
#include "vm/profile/default_profile/firmware/firmware_portal.h"
#include "vm/profile/default_profile/firmware/qddisk.h"

static C_VOID vm_profile_default_firmware_portal_dispatch(C_VOID *opaque,
    core_machine_cpu_execution_context *execution, uint8_t vector)
{
    vm_profile_default_context *profile =
        (vm_profile_default_context *)opaque;

    if (profile == STD_NULL || execution == STD_NULL) return;
    switch (vector) {
    case VM_PROFILE_DEFAULT_PORTAL_HDD_READ:
        vm_profile_default_disk_handle_hdd_read(profile);
        break;
    case VM_PROFILE_DEFAULT_PORTAL_HDD_WRITE:
        vm_profile_default_disk_handle_hdd_write(profile);
        break;
    }
}

type_status vm_profile_default_firmware_portal_install(core_machine *machine,
    vm_profile_default_context *profile, uint32_t origin_linear_start,
    uint32_t origin_linear_bytes)
{
    static const uint8_t vectors[] = {
        VM_PROFILE_DEFAULT_PORTAL_HDD_READ,
        VM_PROFILE_DEFAULT_PORTAL_HDD_WRITE
    };
    core_machine_firmware_interrupt_portal portal;
    STD_SIZE_T index;

    if (machine == STD_NULL || profile == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    portal.origin_linear_start = origin_linear_start;
    portal.origin_linear_bytes = origin_linear_bytes;
    portal.provider = vm_profile_default_firmware_portal_dispatch;
    portal.context = profile;
    for (index = 0u; index < sizeof(vectors) / sizeof(vectors[0]); ++index) {
        portal.vector = vectors[index];
        if (core_machine_install_firmware_interrupt_portal(machine, &portal) !=
            TYPE_STATUS_OK) {
            return TYPE_STATUS_FAULT;
        }
    }
    return TYPE_STATUS_OK;
}
