/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/devices/machine_interface.h"
#include "core/machine/media/media.h"
#include "core/machine/machine_devices.h"
#include "core/machine/machine_private.h"
#include "core/machine/media/fdd.h"
#include "core/machine/media/hdd.h"

type_status vm_machine_devices_initialize_media(vm_machine *session)
{
    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (vm_machine_fdd_initialize_with_geometry(&session->fdd,
            vm_profile_floppy_geometry_get(session->fdd_media_kind))) return TYPE_STATUS_FAULT;
    if (vm_profile_machine_plan_floppy_slot_count(session->profile_plan) > 1u &&
        vm_machine_fdd_initialize_with_geometry(
            &session->floppy[1u], vm_profile_floppy_geometry_get(
                session->fdd_media_kind))) return TYPE_STATUS_FAULT;
    if (vm_profile_machine_plan_hdc_present(session->profile_plan)) {
        vm_machine_hdd_initialize(&session->hdd);
    }
    return TYPE_STATUS_OK;
}

type_status vm_machine_devices_bind_media(vm_machine *session)
{
    type_status status;

    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_media_registry_bind(session->media_registry,
        VM_MACHINE_MEDIA_FDD_ID, &session->fdd, vm_machine_fdd_media_provider());
    if (status != TYPE_STATUS_OK) return status;
    if (vm_profile_machine_plan_floppy_slot_count(session->profile_plan) > 1u) {
        status = core_machine_media_registry_bind(session->media_registry,
            VM_MACHINE_MEDIA_FDD_SECONDARY_ID, &session->floppy[1u],
            vm_machine_fdd_media_provider());
        if (status != TYPE_STATUS_OK) return status;
    }
    if (vm_profile_machine_plan_hdc_present(session->profile_plan)) {
        status = core_machine_media_registry_bind(session->media_registry,
            VM_MACHINE_MEDIA_HDD_ID, &session->hdd, vm_machine_hdd_media_provider());
        if (status != TYPE_STATUS_OK) return status;
    }
    return core_machine_media_registry_freeze(session->media_registry);
}

C_VOID vm_machine_devices_reset(vm_machine *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdd_reset(&session->fdd);
    if (vm_profile_machine_plan_floppy_slot_count(session->profile_plan) > 1u)
        vm_machine_fdd_reset(&session->floppy[1u]);
    if (vm_profile_machine_plan_hdc_present(session->profile_plan)) {
        vm_machine_hdd_reset(&session->hdd);
    }
}

C_VOID vm_machine_devices_finalize(vm_machine *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdd_finalize(&session->fdd);
    if (vm_profile_machine_plan_floppy_slot_count(session->profile_plan) > 1u)
        vm_machine_fdd_finalize(&session->floppy[1u]);
    if (vm_profile_machine_plan_hdc_present(session->profile_plan)) {
        vm_machine_hdd_finalize(&session->hdd);
    }
}
