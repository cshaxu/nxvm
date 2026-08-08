/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/machine_devices.h"
#include "vm/composition/session/session.h"
#include "core/machine/fdc.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "core/machine/hdc.h"

C_VOID vm_session_machine_devices_initialize_media(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdd_initialize(&session->fdd);
    vm_machine_hdd_initialize(&session->hdd);
}

C_VOID vm_session_machine_devices_initialize_fdc(vm_session *session)
{
    const vm_profile_default_pc_at_port_range *ports;
    const vm_profile_default_pc_at_route *route;
    const core_machine_fdc_drive_bindings drives = {
        {VM_SESSION_MEDIA_FDD_ID, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}
    };
    core_machine_fdc_config config;
    core_machine_fdc *fdc;

    if (session == STD_NULL) return;
    fdc = core_machine_configuration_fdc_borrow(session->core_machine);
    if (fdc == STD_NULL) return;
    ports = vm_profile_default_pc_at_port_range_find(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC);
    route = vm_profile_default_pc_at_route_find(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC);
    if (ports == STD_NULL || route == STD_NULL || ports->last - ports->first != 5u) {
        return;
    }
    config.dor_port = ports->first;
    config.status_port = ports->first + 2u;
    config.data_port = ports->first + 3u;
    config.direction_port = ports->last;
    config.control_port = ports->last;
    config.irq = route->irq;
    config.dma_channel = route->dma_channel;
    if (session->fdc_dma_request.core_owner == STD_NULL ||
        session->fdc_dma_request.channel != config.dma_channel) return;
    core_machine_fdc_connect(fdc, &session->media_registry,
        &drives, &session->fdc_dma_request,
        core_machine_configuration_shared_pic_master_borrow(session->core_machine),
        core_machine_configuration_shared_pic_slave_borrow(session->core_machine),
        core_machine_configuration_port_borrow(session->core_machine), &config);
    core_machine_fdc_initialize(fdc);
}

C_INT vm_session_machine_devices_initialize_hdc(vm_session *session)
{
    const vm_profile_default_pc_at_hdc_pio *ports;
    core_machine_hdc_config config;
    core_machine_hdc *hdc;
    const core_machine_port_provider *provider;

    if (session == STD_NULL || session->profile == STD_NULL ||
        session->core_machine == STD_NULL) return 0;
    hdc = core_machine_configuration_hdc_borrow(session->core_machine);
    if (hdc == STD_NULL) return 0;
    ports = &session->profile->hdc_pio;
    if (ports->data_port != 0x01f0u || ports->error_features_port != 0x01f1u ||
        ports->status_command_port != 0x01f7u ||
        ports->alternate_status_device_control_port != 0x03f6u ||
        ports->irq != 14u || ports->dma_channel !=
        VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL || ports->data_width_bits != 16u ||
        ports->register_width_bits != 8u || !ports->lba28_supported ||
        ports->slave_present || ports->secondary_channel_present) return 0;
    config.data_port = ports->data_port;
    config.error_features_port = ports->error_features_port;
    config.sector_count_port = ports->sector_count_port;
    config.sector_number_port = ports->sector_number_port;
    config.cylinder_low_port = ports->cylinder_low_port;
    config.cylinder_high_port = ports->cylinder_high_port;
    config.drive_head_port = ports->drive_head_port;
    config.status_command_port = ports->status_command_port;
    config.alternate_status_device_control_port =
        ports->alternate_status_device_control_port;
    config.irq = ports->irq;
    config.lba28_supported = ports->lba28_supported;
    core_machine_hdc_connect(hdc, &session->media_registry,
        VM_SESSION_MEDIA_HDD_ID,
        core_machine_configuration_shared_pic_master_borrow(session->core_machine),
        core_machine_configuration_shared_pic_slave_borrow(session->core_machine),
        &config);
    core_machine_hdc_initialize(hdc);
    provider = core_machine_hdc_port_provider();
    return core_machine_install_port_provider(session->core_machine,
        ports->data_port, ports->status_command_port, provider, hdc) ==
        TYPE_STATUS_OK && core_machine_install_port_provider(session->core_machine,
        ports->alternate_status_device_control_port,
        ports->alternate_status_device_control_port, provider, hdc) ==
        TYPE_STATUS_OK;
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
