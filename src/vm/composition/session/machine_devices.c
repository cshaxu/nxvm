/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/rtc.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/machine_devices.h"
#include "vm/composition/session/session.h"
#include "core/machine/fdc.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "core/machine/hdc.h"

static type_status vm_session_machine_devices_rtc_read(C_VOID *owner,
    uint16_t port, uint32_t *out_value)
{
    vm_session *session = (vm_session *)owner;
    const vm_profile_default_pc_at_port_range *ports;

    if (session == STD_NULL || out_value == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ports = vm_profile_default_pc_at_port_range_find(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS);
    if (ports == STD_NULL || port != ports->last) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_value = core_machine_rtc_read_selected(&session->rtc);
    return TYPE_STATUS_OK;
}

static type_status vm_session_machine_devices_rtc_write(C_VOID *owner,
    uint16_t port, uint32_t value)
{
    vm_session *session = (vm_session *)owner;
    const vm_profile_default_pc_at_port_range *ports;

    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    ports = vm_profile_default_pc_at_port_range_find(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS);
    if (ports == STD_NULL) return TYPE_STATUS_INVALID_STATE;
    if (port == ports->first) {
        (C_VOID)core_machine_set_nmi_mask(session->core_machine,
            (value & 0x80u) != 0u ? TYPE_TRUE : TYPE_FALSE);
        core_machine_rtc_select_register(&session->rtc, (uint8_t)value);
        return TYPE_STATUS_OK;
    }
    if (port == ports->last) {
        core_machine_rtc_write_selected(&session->rtc, (uint8_t)value);
        return TYPE_STATUS_OK;
    }
    return TYPE_STATUS_INVALID_ARGUMENT;
}

static const core_machine_port_provider vm_session_machine_devices_rtc_provider = {
    vm_session_machine_devices_rtc_read,
    vm_session_machine_devices_rtc_write
};

static C_VOID vm_session_machine_devices_apply_rtc_defaults(vm_session *session)
{
    const vm_profile_default_pc_at_cmos_defaults *defaults;

    if (session == STD_NULL || session->profile == STD_NULL) return;
    defaults = &session->profile->cmos;
    core_machine_rtc_write_nvram(&session->rtc, CORE_MACHINE_RTC_TYPE_DISK_FLOPPY,
        defaults->floppy_type);
    core_machine_rtc_write_nvram(&session->rtc, CORE_MACHINE_RTC_TYPE_DISK_FIXED,
        defaults->fixed_disk_type);
    core_machine_rtc_write_nvram(&session->rtc, CORE_MACHINE_RTC_EQUIPMENT,
        defaults->equipment);
    core_machine_rtc_write_nvram(&session->rtc, CORE_MACHINE_RTC_BASEMEM_LSB,
        TYPE_MASK_UNSIGNED_8(defaults->base_memory_kib));
    core_machine_rtc_write_nvram(&session->rtc, CORE_MACHINE_RTC_BASEMEM_MSB,
        TYPE_MASK_UNSIGNED_8(defaults->base_memory_kib >> 8));
}

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
    core_machine_rtc_config config;

    if (session == STD_NULL) return;
    ports = vm_profile_default_pc_at_port_range_find(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS);
    route = vm_profile_default_pc_at_route_find(session->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS);
    if (ports == STD_NULL || route == STD_NULL || ports->last - ports->first != 1u) return;
    config.irq = route->irq;
    config.ticks_per_second = session->profile->rtc_ticks_per_second;
    core_machine_rtc_initialize(&session->rtc,
        core_machine_configuration_shared_pic_master_borrow(session->core_machine),
        core_machine_configuration_shared_pic_slave_borrow(session->core_machine), &config);
    vm_session_machine_devices_apply_rtc_defaults(session);
    (C_VOID)core_machine_install_port_provider(session->core_machine, ports->first,
        ports->last, &vm_session_machine_devices_rtc_provider, session);
}

C_VOID vm_session_machine_devices_initialize_fdc(vm_session *session)
{
    const vm_profile_default_pc_at_port_range *ports;
    const vm_profile_default_pc_at_route *route;
    core_machine_dma_request_binding dma_request = {0};
    core_machine_fdc_config config;
    core_machine_fdc *fdc;

    if (session == STD_NULL) return;
    fdc = core_machine_configuration_shared_fdc_borrow(session->core_machine);
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
    if (core_machine_dma_bind_channel(
        core_machine_configuration_shared_dma_latch_borrow(session->core_machine),
        core_machine_configuration_shared_dma_primary_borrow(session->core_machine),
        core_machine_configuration_shared_dma_secondary_borrow(session->core_machine),
        config.dma_channel, core_machine_fdc_dma_provider(), fdc,
        &dma_request) != TYPE_STATUS_OK) return;
    core_machine_fdc_connect(fdc, &session->media_registry,
        VM_SESSION_MEDIA_FDD_ID, &dma_request,
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
    hdc = core_machine_configuration_shared_hdc_borrow(session->core_machine);
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

C_VOID vm_session_machine_devices_reset_cmos(vm_session *session)
{
    if (session != STD_NULL) core_machine_rtc_reset(&session->rtc);
}

C_VOID vm_session_machine_devices_advance(vm_session *session,
    uint64_t elapsed_ticks)
{
    if (session != STD_NULL) core_machine_rtc_advance(&session->rtc, elapsed_ticks);
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
    vm_session_machine_devices_reset_cmos(session);
    vm_machine_fdd_reset(&session->fdd);
    vm_machine_hdd_reset(&session->hdd);
}

C_VOID vm_session_machine_devices_finalize(vm_session *session)
{
    if (session == STD_NULL) return;
    core_machine_rtc_finalize(&session->rtc);
    vm_machine_fdd_finalize(&session->fdd);
    vm_machine_hdd_finalize(&session->hdd);
}
