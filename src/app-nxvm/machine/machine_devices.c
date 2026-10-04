/* Copyright 2012-2014 Neko. */
#include "lib/types/types_interface.h"


#include "x86/core/machine_interface.h"
#include "x86/product/machine/media/media_interface.h"
#include "app-nxvm/machine/machine_devices.h"
#include "app-nxvm/machine/machine_private.h"
#include "x86/product/machine/media/fdd_interface.h"
#include "x86/product/machine/media/hdd_interface.h"

lib_status vm_machine_devices_initialize_media(vm_machine *session)
{
    if (session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (vm_machine_fdd_allocate(vm_profile_floppy_geometry_get(
            session->fdd_media_kind), &session->fdd) != LIB_STATUS_OK)
        return LIB_STATUS_INTERNAL_ERROR;
    if (vm_profile_machine_plan_floppy_slot_count(session->profile_plan) > 1u &&
        vm_machine_fdd_allocate(vm_profile_floppy_geometry_get(
            session->fdd_media_kind), &session->floppy[1u]) != LIB_STATUS_OK)
        return LIB_STATUS_INTERNAL_ERROR;
    if (vm_profile_machine_plan_hdc_present(session->profile_plan)) {
        if (vm_machine_hdd_allocate(&session->hdd) != LIB_STATUS_OK)
            return LIB_STATUS_INTERNAL_ERROR;
    }
    return LIB_STATUS_OK;
}

lib_status vm_machine_devices_bind_media(vm_machine *session)
{
    lib_status status;

    if (session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = core_machine_media_registry_bind(session->media_registry,
        VM_MACHINE_MEDIA_FDD_ID, session->fdd, vm_machine_fdd_media_provider());
    if (status != LIB_STATUS_OK) return status;
    if (vm_profile_machine_plan_floppy_slot_count(session->profile_plan) > 1u) {
        status = core_machine_media_registry_bind(session->media_registry,
            VM_MACHINE_MEDIA_FDD_SECONDARY_ID, session->floppy[1u],
            vm_machine_fdd_media_provider());
        if (status != LIB_STATUS_OK) return status;
    }
    if (vm_profile_machine_plan_hdc_present(session->profile_plan)) {
        status = core_machine_media_registry_bind(session->media_registry,
            VM_MACHINE_MEDIA_HDD_ID, session->hdd, vm_machine_hdd_media_provider());
        if (status != LIB_STATUS_OK) return status;
    }
    return core_machine_media_registry_freeze(session->media_registry);
}

void vm_machine_devices_reset(vm_machine *session)
{
    if (session == LIB_NULL) return;
    vm_machine_fdd_reset(session->fdd);
    if (vm_profile_machine_plan_floppy_slot_count(session->profile_plan) > 1u)
        vm_machine_fdd_reset(session->floppy[1u]);
    if (vm_profile_machine_plan_hdc_present(session->profile_plan)) {
        vm_machine_hdd_reset(session->hdd);
    }
}

void vm_machine_devices_finalize(vm_machine *session)
{
    if (session == LIB_NULL) return;
    /* Empty slots also participate in partial-construction rollback. */
    for (lib_size slot = 0u; slot < VM_MACHINE_FLOPPY_SLOT_COUNT; ++slot)
        vm_machine_fdd_destroy(&session->floppy[slot]);
    for (lib_size slot = 0u; slot < VM_MACHINE_FIXED_DISK_SLOT_COUNT; ++slot)
        vm_machine_hdd_destroy(&session->fixed_disk[slot]);
}
