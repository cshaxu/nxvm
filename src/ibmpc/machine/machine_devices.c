/* Copyright 2012-2014 Neko. */
#include "lib/types/types_interface.h"


#include "x86/core/machine_interface.h"
#include "ibmpc/machine/media/media_interface.h"
#include "ibmpc/machine/machine_devices.h"
#include "ibmpc/machine/machine_private.h"
#include "ibmpc/machine/media/fdd_interface.h"
#include "ibmpc/machine/media/hdd_interface.h"

lib_status vm_machine_devices_initialize_media(vm_machine *session)
{
    lib_status status;

    if (session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_machine_fdd_allocate(vm_profile_floppy_geometry_get(
        session->construction.media_kind), &session->floppy[0u]);
    if (status != LIB_STATUS_OK) return status;
    if (session->construction.floppy_slot_count > 1u) {
        status = vm_machine_fdd_allocate(vm_profile_floppy_geometry_get(
            session->construction.media_kind), &session->floppy[1u]);
        if (status != LIB_STATUS_OK) return status;
    }
    if (session->construction.hdc_present) {
        status = vm_machine_hdd_allocate(&session->fixed_disk[0u]);
        if (status != LIB_STATUS_OK) return status;
    }
    return LIB_STATUS_OK;
}

lib_status vm_machine_devices_bind_media(vm_machine *session)
{
    lib_status status;

    if (session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = core_machine_media_registry_bind(session->media_registry,
        VM_MACHINE_MEDIA_FDD_ID, session->floppy[0u], vm_machine_fdd_media_provider());
    if (status != LIB_STATUS_OK) return status;
    if (session->construction.floppy_slot_count > 1u) {
        status = core_machine_media_registry_bind(session->media_registry,
            VM_MACHINE_MEDIA_FDD_SECONDARY_ID, session->floppy[1u],
            vm_machine_fdd_media_provider());
        if (status != LIB_STATUS_OK) return status;
    }
    if (session->construction.hdc_present) {
        status = core_machine_media_registry_bind(session->media_registry,
            VM_MACHINE_MEDIA_HDD_ID, session->fixed_disk[0u], vm_machine_hdd_media_provider());
        if (status != LIB_STATUS_OK) return status;
    }
    return core_machine_media_registry_freeze(session->media_registry);
}

void vm_machine_devices_reset(vm_machine *session)
{
    if (session == LIB_NULL) return;
    vm_machine_fdd_reset(session->floppy[0u]);
    if (session->construction.floppy_slot_count > 1u)
        vm_machine_fdd_reset(session->floppy[1u]);
    if (session->construction.hdc_present) {
        vm_machine_hdd_reset(session->fixed_disk[0u]);
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
