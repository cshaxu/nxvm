/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/machine_devices.h"
#include "vm/composition/session/session.h"
#include "vm/machine/cmos.h"
#include "vm/machine/fdc.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/machine/hdc.h"

C_VOID vm_session_machine_devices_initialize_media(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdd_initialize(&session->fdd);
    vm_machine_hdd_initialize(&session->hdd);
}

C_VOID vm_session_machine_devices_initialize_cmos(vm_session *session)
{
    const vm_profile_default_pc_at_port_range *ports;
    const vm_profile_default_pc_at_route *route;
    vm_machine_cmos_config config;

    if (session == STD_NULL) return;
    ports = vm_profile_default_pc_at_port_range_find(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS);
    route = vm_profile_default_pc_at_route_find(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS);
    if (ports == STD_NULL || route == STD_NULL || ports->last - ports->first != 1u) return;
    config.index_port = ports->first;
    config.data_port = ports->last;
    config.irq = route->irq;
    config.ticks_per_second = session->profile->rtc_ticks_per_second;
    vm_machine_cmos_initialize(&session->cmos,
        core_machine_configuration_cpu_borrow(session->core_machine),
        core_machine_configuration_shared_pic_master_borrow(session->core_machine),
        core_machine_configuration_shared_pic_slave_borrow(session->core_machine),
        core_machine_configuration_port_borrow(session->core_machine), &config);
}

C_VOID vm_session_machine_devices_initialize_fdc(vm_session *session)
{
    const vm_profile_default_pc_at_port_range *ports;
    const vm_profile_default_pc_at_route *route;
    core_machine_dma_request_binding dma_request = {0};
    vm_machine_fdc_config config;

    if (session == STD_NULL) return;
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
    if (core_machine_dma_bind_channel(
        core_machine_configuration_shared_dma_latch_borrow(session->core_machine),
        core_machine_configuration_shared_dma_primary_borrow(session->core_machine),
        core_machine_configuration_shared_dma_secondary_borrow(session->core_machine),
        config.dma_channel, vm_machine_fdc_dma_provider(), &session->fdc,
        &dma_request) != TYPE_STATUS_OK) return;
    vm_machine_fdc_connect(&session->fdc, &session->fdd, &dma_request,
        core_machine_configuration_shared_pic_master_borrow(session->core_machine),
        core_machine_configuration_shared_pic_slave_borrow(session->core_machine),
        core_machine_configuration_port_borrow(session->core_machine), &config);
    vm_machine_fdc_initialize(&session->fdc);
}

C_INT vm_session_machine_devices_initialize_hdc(vm_session *session)
{
    const vm_profile_default_pc_at_hdc_pio *ports;
    vm_machine_hdc_config config;
    const core_machine_port_provider *provider;

    if (session == STD_NULL || session->profile == STD_NULL ||
        session->core_machine == STD_NULL) return 0;
    ports = &session->profile->hdc_pio;
    if (ports->data_port != 0x01f0u || ports->error_features_port != 0x01f1u ||
        ports->status_command_port != 0x01f7u ||
        ports->alternate_status_device_control_port != 0x03f6u ||
        ports->irq != 14u || ports->dma_channel !=
        VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL || ports->data_width_bits != 16u ||
        ports->register_width_bits != 8u) return 0;
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
    vm_machine_hdc_connect(&session->hdc, &session->hdd,
        core_machine_configuration_shared_pic_master_borrow(session->core_machine),
        core_machine_configuration_shared_pic_slave_borrow(session->core_machine),
        &config);
    vm_machine_hdc_initialize(&session->hdc);
    provider = vm_machine_hdc_port_provider();
    return core_machine_install_port_provider(session->core_machine,
        ports->data_port, ports->status_command_port, provider, &session->hdc) ==
        TYPE_STATUS_OK && core_machine_install_port_provider(session->core_machine,
        ports->alternate_status_device_control_port,
        ports->alternate_status_device_control_port, provider, &session->hdc) ==
        TYPE_STATUS_OK;
}

C_VOID vm_session_machine_devices_reset_cmos(vm_session *session)
{
    if (session != STD_NULL) vm_machine_cmos_reset(&session->cmos);
}

C_VOID vm_session_machine_devices_refresh_cmos(vm_session *session)
{
    if (session != STD_NULL) vm_machine_cmos_refresh(&session->cmos);
}

C_VOID vm_session_machine_devices_advance(vm_session *session,
    uint64_t elapsed_ticks)
{
    if (session != STD_NULL) vm_machine_cmos_advance(&session->cmos, elapsed_ticks);
}

C_VOID vm_session_machine_devices_refresh(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdd_refresh(&session->fdd);
    vm_machine_hdd_refresh(&session->hdd);
    vm_machine_hdc_refresh(&session->hdc);
    vm_machine_cmos_refresh(&session->cmos);
    vm_machine_fdc_refresh(&session->fdc);
}

C_VOID vm_session_machine_devices_reset(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_machine_devices_reset_cmos(session);
    vm_machine_fdc_reset(&session->fdc);
    vm_machine_fdd_reset(&session->fdd);
    vm_machine_hdd_reset(&session->hdd);
    vm_machine_hdc_reset(&session->hdc);
}

C_VOID vm_session_machine_devices_finalize(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_cmos_finalize(&session->cmos);
    vm_machine_fdc_finalize(&session->fdc);
    vm_machine_hdc_finalize(&session->hdc);
    vm_machine_fdd_finalize(&session->fdd);
    vm_machine_hdd_finalize(&session->hdd);
}
