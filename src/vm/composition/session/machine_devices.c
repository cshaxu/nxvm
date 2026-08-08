/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/machine_devices.h"
#include "vm/composition/session/session.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"

C_VOID vm_session_machine_devices_initialize_media(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdd_initialize(&session->fdd);
    vm_machine_hdd_initialize(&session->hdd);
}

static type_status vm_session_machine_devices_configure_fdc(vm_session *session)
{
    const vm_profile_default_pc_at_port_range *ports;
    const vm_profile_default_pc_at_route *route;
    const core_machine_fdc_drive_bindings drives = {
        {VM_SESSION_MEDIA_FDD_ID, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}
    };
    core_machine_fdc_topology topology = {0};

    if (session == STD_NULL || session->core_machine == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ports = vm_profile_default_pc_at_port_range_find(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC);
    route = vm_profile_default_pc_at_route_find(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC);
    if (ports == STD_NULL || route == STD_NULL || ports->last - ports->first != 5u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    topology.media_registry = &session->media_registry;
    topology.drives = drives;
    topology.dma_request = session->fdc_dma_request;
    topology.config.dor_port = ports->first;
    topology.config.status_port = ports->first + 2u;
    topology.config.data_port = ports->first + 3u;
    topology.config.direction_port = ports->last;
    topology.config.control_port = ports->last;
    topology.config.irq = route->irq;
    topology.config.dma_channel = route->dma_channel;
    return core_machine_configure_fdc(session->core_machine, &topology);
}

static type_status vm_session_machine_devices_configure_hdc(vm_session *session)
{
    const vm_profile_default_pc_at_hdc_pio *ports;
    core_machine_hdc_topology topology = {0};

    if (session == STD_NULL || session->profile == STD_NULL ||
        session->core_machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    ports = &session->profile->hdc_pio;
    if (ports->data_port != 0x01f0u || ports->error_features_port != 0x01f1u ||
        ports->status_command_port != 0x01f7u ||
        ports->alternate_status_device_control_port != 0x03f6u ||
        ports->irq != 14u || ports->dma_channel !=
        VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL || ports->data_width_bits != 16u ||
        ports->register_width_bits != 8u || !ports->lba28_supported ||
        ports->slave_present || ports->secondary_channel_present) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    topology.media_registry = &session->media_registry;
    topology.media_id = VM_SESSION_MEDIA_HDD_ID;
    topology.config.data_port = ports->data_port;
    topology.config.error_features_port = ports->error_features_port;
    topology.config.sector_count_port = ports->sector_count_port;
    topology.config.sector_number_port = ports->sector_number_port;
    topology.config.cylinder_low_port = ports->cylinder_low_port;
    topology.config.cylinder_high_port = ports->cylinder_high_port;
    topology.config.drive_head_port = ports->drive_head_port;
    topology.config.status_command_port = ports->status_command_port;
    topology.config.alternate_status_device_control_port =
        ports->alternate_status_device_control_port;
    topology.config.irq = ports->irq;
    topology.config.lba28_supported = ports->lba28_supported;
    return core_machine_configure_hdc(session->core_machine, &topology);
}

type_status vm_session_machine_devices_configure_controllers(vm_session *session)
{
    type_status status = vm_session_machine_devices_configure_fdc(session);

    if (status != TYPE_STATUS_OK) return status;
    return vm_session_machine_devices_configure_hdc(session);
}

C_VOID vm_session_machine_devices_refresh(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdd_refresh(&session->fdd);
    vm_machine_hdd_refresh(&session->hdd);
}

C_VOID vm_session_machine_devices_reset(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdd_reset(&session->fdd);
    vm_machine_hdd_reset(&session->hdd);
}

C_VOID vm_session_machine_devices_finalize(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdd_finalize(&session->fdd);
    vm_machine_hdd_finalize(&session->hdd);
}
