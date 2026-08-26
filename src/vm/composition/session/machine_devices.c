/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/machine_devices.h"
#include "vm/composition/session/session_private.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"

type_status vm_session_machine_devices_initialize_media(vm_session *session)
{
    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (vm_machine_fdd_initialize_with_geometry(&session->fdd,
            vm_profile_floppy_geometry_get(session->floppy_kind))) return TYPE_STATUS_FAULT;
    if (session->model40_private || session->xt_private ||
        (session->profile != STD_NULL && session->profile->hdc_present)) {
        vm_machine_hdd_initialize(&session->hdd);
    }
    return TYPE_STATUS_OK;
}

static type_status vm_session_machine_devices_materialize_fdc(vm_session *session,
    core_machine_plan *plan)
{
    const vm_profile_default_pc_at_port_leaf *dor_port;
    const vm_profile_default_pc_at_port_leaf *status_port;
    const vm_profile_default_pc_at_port_leaf *data_port;
    const vm_profile_default_pc_at_port_leaf *control_port;
    const vm_profile_default_pc_at_route *route;
    const core_machine_fdc_drive_bindings drives = {
        {VM_SESSION_MEDIA_FDD_ID, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}
    };
    core_machine_fdc_config config = {0};

    if (session == STD_NULL || plan == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    dor_port = vm_profile_default_pc_at_port_leaf_at(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 0u);
    status_port = vm_profile_default_pc_at_port_leaf_at(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 1u);
    data_port = vm_profile_default_pc_at_port_leaf_at(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 2u);
    control_port = vm_profile_default_pc_at_port_leaf_at(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 3u);
    route = vm_profile_default_pc_at_route_find(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_FDC_IRQ6_DMA2);
    if (dor_port == STD_NULL || status_port == STD_NULL || data_port == STD_NULL ||
        control_port == STD_NULL || route == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    config.dor_port = dor_port->port;
    config.status_port = status_port->port;
    config.data_port = data_port->port;
    config.direction_port = control_port->port;
    config.control_port = control_port->port;
    config.irq = route->irq;
    config.dma_channel = route->dma_channel;
    return core_machine_plan_configure_fdc(plan, &drives, &config);
}

static type_status vm_session_machine_devices_materialize_hdc(vm_session *session,
    core_machine_plan *plan)
{
    if (session == STD_NULL || session->profile == STD_NULL || plan == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (!session->profile->hdc_present) return TYPE_STATUS_OK;
    if (!vm_profile_default_pc_at_descriptor_is_valid(session->profile)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return core_machine_plan_configure_hdc(plan, VM_SESSION_MEDIA_HDD_ID,
        CORE_MACHINE_MEDIA_ID_INVALID, &session->profile->hdc);
}

type_status vm_session_machine_devices_materialize_plan(vm_session *session,
    core_machine_plan *plan)
{
    type_status status;

    if (session == STD_NULL || plan == STD_NULL || session->model40_private) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    /* XT is the only resolved topology that already carries both sourced
     * storage devices.  PC/AT profiles retain their established descriptor
     * materialization route. */
    if (session->xt_private) return TYPE_STATUS_OK;
    if (!vm_profile_default_pc_at_descriptor_is_valid(session->profile)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = vm_session_machine_devices_materialize_fdc(session, plan);

    if (status != TYPE_STATUS_OK) return status;
    return vm_session_machine_devices_materialize_hdc(session, plan);
}

C_VOID vm_session_machine_devices_refresh(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdd_refresh(&session->fdd);
    if (session->model40_private || session->xt_private ||
        (session->profile != STD_NULL && session->profile->hdc_present)) {
        vm_machine_hdd_refresh(&session->hdd);
    }
}

C_VOID vm_session_machine_devices_reset(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdd_reset(&session->fdd);
    if (session->model40_private || session->xt_private ||
        (session->profile != STD_NULL && session->profile->hdc_present)) {
        vm_machine_hdd_reset(&session->hdd);
    }
}

C_VOID vm_session_machine_devices_finalize(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdd_finalize(&session->fdd);
    if (session->model40_private || session->xt_private ||
        (session->profile != STD_NULL && session->profile->hdc_present)) {
        vm_machine_hdd_finalize(&session->hdd);
    }
}
