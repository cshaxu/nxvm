#include "type.h"

#include "core/machine/media_interface.h"
#include "core/machine/pic.h"
#include "vm/machine/hdc.h"

#define VM_MACHINE_HDC_COMMAND_READ_SECTORS 0x20u
#define VM_MACHINE_HDC_COMMAND_WRITE_SECTORS 0x30u
#define VM_MACHINE_HDC_COMMAND_IDENTIFY_DEVICE 0xecu
#define VM_MACHINE_HDC_DEVICE_CONTROL_SRST 0x04u

static C_INT vm_machine_hdc_selected_master(const vm_machine_hdc *hdc)
{ return hdc != STD_NULL && (hdc->data.drive_head & 0x10u) == 0u; }

static C_INT vm_machine_hdc_lba_mode(const vm_machine_hdc *hdc)
{
    return hdc != STD_NULL && hdc->connect.config.lba28_supported &&
        (hdc->data.drive_head & 0x40u) != 0u;
}

static C_VOID vm_machine_hdc_clear_irq(vm_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    hdc->data.irq_pending = TYPE_FALSE;
    core_machine_pic_irq_source_deassert(&hdc->connect.irq_source);
}

static C_VOID vm_machine_hdc_raise_irq(vm_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    hdc->data.irq_pending = TYPE_TRUE;
    core_machine_pic_irq_source_assert(&hdc->connect.irq_source);
}

static uint32_t vm_machine_hdc_lba(const vm_machine_hdc *hdc)
{
    return (uint32_t)hdc->data.sector_number |
        ((uint32_t)hdc->data.cylinder_low << 8u) |
        ((uint32_t)hdc->data.cylinder_high << 16u) |
        ((uint32_t)(hdc->data.drive_head & 0x0fu) << 24u);
}

static C_INT vm_machine_hdc_media_info(const vm_machine_hdc *hdc,
    core_machine_media_info *out_info, core_machine_media_result *out_result)
{
    return hdc != STD_NULL && hdc->connect.media_registry != STD_NULL &&
        core_machine_media_query(hdc->connect.media_registry, hdc->connect.media_id,
            out_info, out_result) == TYPE_STATUS_OK &&
        *out_result == CORE_MACHINE_MEDIA_RESULT_OK;
}

static STD_SIZE_T vm_machine_hdc_sector_capacity(
    const core_machine_media_info *info)
{
    return info == STD_NULL || info->geometry.bytes_per_sector == 0u ? 0u :
        (STD_SIZE_T)info->geometry.logical_sector_count;
}

static C_VOID vm_machine_hdc_complete(vm_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    hdc->data.phase = VM_MACHINE_HDC_PHASE_IDLE;
    hdc->data.data_index = 0u;
    hdc->data.status = VM_MACHINE_HDC_STATUS_DRDY | VM_MACHINE_HDC_STATUS_DSC;
    vm_machine_hdc_raise_irq(hdc);
}

static C_VOID vm_machine_hdc_fail(vm_machine_hdc *hdc, uint8_t error)
{
    if (hdc == STD_NULL) return;
    hdc->data.error = error;
    hdc->data.phase = VM_MACHINE_HDC_PHASE_IDLE;
    hdc->data.data_index = 0u;
    hdc->data.status = VM_MACHINE_HDC_STATUS_DRDY | VM_MACHINE_HDC_STATUS_ERR;
    vm_machine_hdc_raise_irq(hdc);
}

static C_INT vm_machine_hdc_load_chs_sector(vm_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    uint16_t cylinder;
    uint8_t head;
    uint8_t sector;
    STD_SIZE_T offset;

    if (hdc == STD_NULL || !vm_machine_hdc_media_info(hdc, &info, &media_result) ||
        !info.present) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    cylinder = (uint16_t)hdc->data.cylinder_low |
        ((uint16_t)hdc->data.cylinder_high << 8u);
    head = hdc->data.drive_head & 0x0fu;
    sector = hdc->data.sector_number;
    if (!vm_machine_hdc_selected_master(hdc) || vm_machine_hdc_lba_mode(hdc) ||
        sector == 0u || cylinder >= info.geometry.cylinders ||
        head >= info.geometry.heads || sector > info.geometry.sectors_per_track ||
        info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    offset = (((STD_SIZE_T)cylinder * info.geometry.heads + head) *
        info.geometry.sectors_per_track + (sector - 1u)) * info.geometry.bytes_per_sector;
    if (core_machine_media_read_bytes(hdc->connect.media_registry,
            hdc->connect.media_id, offset, hdc->data.data, sizeof(hdc->data.data),
            &media_result) != TYPE_STATUS_OK ||
        media_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        vm_machine_hdc_fail(hdc, media_result == CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE ?
            VM_MACHINE_HDC_ERROR_ID_NOT_FOUND : VM_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    return 1;
}

static C_INT vm_machine_hdc_load_lba_sector(vm_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    uint32_t lba;
    STD_SIZE_T offset;

    if (hdc == STD_NULL || !vm_machine_hdc_media_info(hdc, &info, &media_result) ||
        !info.present) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    lba = vm_machine_hdc_lba(hdc);
    if (!vm_machine_hdc_selected_master(hdc) || !vm_machine_hdc_lba_mode(hdc) ||
        (STD_SIZE_T)lba >= vm_machine_hdc_sector_capacity(&info) ||
        info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    offset = (STD_SIZE_T)lba * info.geometry.bytes_per_sector;
    if (core_machine_media_read_bytes(hdc->connect.media_registry,
            hdc->connect.media_id, offset, hdc->data.data, sizeof(hdc->data.data),
            &media_result) != TYPE_STATUS_OK ||
        media_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        vm_machine_hdc_fail(hdc, media_result == CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE ?
            VM_MACHINE_HDC_ERROR_ID_NOT_FOUND : VM_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    return 1;
}

static C_INT vm_machine_hdc_load_sector(vm_machine_hdc *hdc)
{
    return vm_machine_hdc_lba_mode(hdc) ? vm_machine_hdc_load_lba_sector(hdc) :
        vm_machine_hdc_load_chs_sector(hdc);
}

static C_INT vm_machine_hdc_store_chs_sector(vm_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    uint16_t cylinder;
    uint8_t head;
    uint8_t sector;
    STD_SIZE_T offset;

    if (hdc == STD_NULL || !vm_machine_hdc_media_info(hdc, &info, &media_result) ||
        !info.present || (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY) != 0u) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    cylinder = (uint16_t)hdc->data.cylinder_low |
        ((uint16_t)hdc->data.cylinder_high << 8u);
    head = hdc->data.drive_head & 0x0fu;
    sector = hdc->data.sector_number;
    if (!vm_machine_hdc_selected_master(hdc) || vm_machine_hdc_lba_mode(hdc) ||
        sector == 0u || cylinder >= info.geometry.cylinders ||
        head >= info.geometry.heads || sector > info.geometry.sectors_per_track ||
        info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    offset = (((STD_SIZE_T)cylinder * info.geometry.heads + head) *
        info.geometry.sectors_per_track + (sector - 1u)) * info.geometry.bytes_per_sector;
    if (core_machine_media_write_bytes(hdc->connect.media_registry,
            hdc->connect.media_id, offset, hdc->data.data, sizeof(hdc->data.data),
            &media_result) != TYPE_STATUS_OK ||
        media_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        vm_machine_hdc_fail(hdc, media_result == CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE ?
            VM_MACHINE_HDC_ERROR_ID_NOT_FOUND : VM_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    return 1;
}

static C_INT vm_machine_hdc_store_lba_sector(vm_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    uint32_t lba;
    STD_SIZE_T offset;

    if (hdc == STD_NULL || !vm_machine_hdc_media_info(hdc, &info, &media_result) ||
        !info.present || (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY) != 0u) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    lba = vm_machine_hdc_lba(hdc);
    if (!vm_machine_hdc_selected_master(hdc) || !vm_machine_hdc_lba_mode(hdc) ||
        (STD_SIZE_T)lba >= vm_machine_hdc_sector_capacity(&info) ||
        info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    offset = (STD_SIZE_T)lba * info.geometry.bytes_per_sector;
    if (core_machine_media_write_bytes(hdc->connect.media_registry,
            hdc->connect.media_id, offset, hdc->data.data, sizeof(hdc->data.data),
            &media_result) != TYPE_STATUS_OK ||
        media_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        vm_machine_hdc_fail(hdc, media_result == CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE ?
            VM_MACHINE_HDC_ERROR_ID_NOT_FOUND : VM_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    return 1;
}

static C_INT vm_machine_hdc_store_sector(vm_machine_hdc *hdc)
{
    return vm_machine_hdc_lba_mode(hdc) ? vm_machine_hdc_store_lba_sector(hdc) :
        vm_machine_hdc_store_chs_sector(hdc);
}

static C_VOID vm_machine_hdc_identify(vm_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    uint16_t word;

    if (hdc == STD_NULL || !vm_machine_hdc_media_info(hdc, &info, &media_result) ||
        !vm_machine_hdc_selected_master(hdc) ||
        !info.present || info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ABORT);
        return;
    }
    STD_MEMSET(hdc->data.data, 0, sizeof(hdc->data.data));
    word = 0x0040u;
    STD_MEMCPY(&hdc->data.data[0], &word, sizeof(word));
    word = info.geometry.cylinders;
    STD_MEMCPY(&hdc->data.data[2], &word, sizeof(word));
    word = info.geometry.heads;
    STD_MEMCPY(&hdc->data.data[6], &word, sizeof(word));
    word = info.geometry.sectors_per_track;
    STD_MEMCPY(&hdc->data.data[12], &word, sizeof(word));
    word = 0x0200u;
    STD_MEMCPY(&hdc->data.data[98], &word, sizeof(word));
    word = (uint16_t)vm_machine_hdc_sector_capacity(&info);
    STD_MEMCPY(&hdc->data.data[120], &word, sizeof(word));
    word = (uint16_t)(vm_machine_hdc_sector_capacity(&info) >> 16u);
    STD_MEMCPY(&hdc->data.data[122], &word, sizeof(word));
    hdc->data.phase = VM_MACHINE_HDC_PHASE_DATA_READ;
    hdc->data.data_index = 0u;
    hdc->data.error = 0u;
    hdc->data.status = VM_MACHINE_HDC_STATUS_DRDY | VM_MACHINE_HDC_STATUS_DSC |
        VM_MACHINE_HDC_STATUS_DRQ;
    vm_machine_hdc_raise_irq(hdc);
}

static C_INT vm_machine_hdc_advance_chs(vm_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    uint16_t cylinder;
    uint8_t head;
    uint8_t sector;

    if (hdc == STD_NULL || !vm_machine_hdc_media_info(hdc, &info, &media_result) ||
        !info.present) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    cylinder = (uint16_t)hdc->data.cylinder_low |
        ((uint16_t)hdc->data.cylinder_high << 8u);
    head = hdc->data.drive_head & 0x0fu;
    sector = (uint8_t)(hdc->data.sector_number + 1u);
    if (sector > info.geometry.sectors_per_track) {
        sector = 1u;
        ++head;
        if (head >= info.geometry.heads) {
            head = 0u;
            ++cylinder;
        }
    }
    if (cylinder >= info.geometry.cylinders) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    hdc->data.sector_number = sector;
    hdc->data.cylinder_low = (uint8_t)cylinder;
    hdc->data.cylinder_high = (uint8_t)(cylinder >> 8u);
    hdc->data.drive_head = (hdc->data.drive_head & 0xf0u) | head;
    return 1;
}

static C_INT vm_machine_hdc_advance_lba(vm_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    uint32_t lba;

    if (hdc == STD_NULL || !vm_machine_hdc_media_info(hdc, &info, &media_result) ||
        !info.present) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    lba = vm_machine_hdc_lba(hdc) + 1u;
    if ((STD_SIZE_T)lba >= vm_machine_hdc_sector_capacity(&info)) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    hdc->data.sector_number = (uint8_t)lba;
    hdc->data.cylinder_low = (uint8_t)(lba >> 8u);
    hdc->data.cylinder_high = (uint8_t)(lba >> 16u);
    hdc->data.drive_head = (hdc->data.drive_head & 0xf0u) |
        (uint8_t)(lba >> 24u);
    return 1;
}

static C_INT vm_machine_hdc_advance_sector(vm_machine_hdc *hdc)
{
    return vm_machine_hdc_lba_mode(hdc) ? vm_machine_hdc_advance_lba(hdc) :
        vm_machine_hdc_advance_chs(hdc);
}

static C_VOID vm_machine_hdc_complete_data_sector(vm_machine_hdc *hdc)
{
    if (hdc == STD_NULL || hdc->data.sectors_remaining == 0u) return;
    --hdc->data.sectors_remaining;
    --hdc->data.sector_count;
}

static C_VOID vm_machine_hdc_next_read_sector(vm_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    vm_machine_hdc_complete_data_sector(hdc);
    if (hdc->data.sectors_remaining == 0u) {
        vm_machine_hdc_complete(hdc);
        return;
    }
    if (!vm_machine_hdc_advance_sector(hdc) || !vm_machine_hdc_load_sector(hdc)) {
        return;
    }
    hdc->data.data_index = 0u;
    vm_machine_hdc_raise_irq(hdc);
}

static C_VOID vm_machine_hdc_next_write_sector(vm_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    if (!vm_machine_hdc_store_sector(hdc)) return;
    vm_machine_hdc_complete_data_sector(hdc);
    if (hdc->data.sectors_remaining == 0u) {
        vm_machine_hdc_complete(hdc);
        return;
    }
    if (!vm_machine_hdc_advance_sector(hdc)) return;
    hdc->data.data_index = 0u;
    STD_MEMSET(hdc->data.data, 0, sizeof(hdc->data.data));
    vm_machine_hdc_raise_irq(hdc);
}

static C_VOID vm_machine_hdc_execute_command(vm_machine_hdc *hdc, uint8_t command)
{
    if (hdc == STD_NULL) return;
    hdc->data.last_command = command;
    ++hdc->data.command_count;
    hdc->data.error = 0u;
    vm_machine_hdc_clear_irq(hdc);
    if (hdc->data.reset_asserted) return;
    if (!vm_machine_hdc_selected_master(hdc)) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ABORT);
        return;
    }
    if ((hdc->data.drive_head & 0x40u) != 0u &&
        !hdc->connect.config.lba28_supported) {
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ABORT);
        return;
    }
    switch (command) {
    case VM_MACHINE_HDC_COMMAND_IDENTIFY_DEVICE:
        vm_machine_hdc_identify(hdc);
        break;
    case VM_MACHINE_HDC_COMMAND_READ_SECTORS:
        if (!vm_machine_hdc_load_sector(hdc)) break;
        hdc->data.phase = VM_MACHINE_HDC_PHASE_DATA_READ;
        hdc->data.sectors_remaining = hdc->data.sector_count == 0u ? 256u :
            hdc->data.sector_count;
        hdc->data.data_index = 0u;
        hdc->data.status = VM_MACHINE_HDC_STATUS_DRDY | VM_MACHINE_HDC_STATUS_DSC |
            VM_MACHINE_HDC_STATUS_DRQ;
        vm_machine_hdc_raise_irq(hdc);
        break;
    case VM_MACHINE_HDC_COMMAND_WRITE_SECTORS:
        if (!vm_machine_hdc_load_sector(hdc)) break;
        hdc->data.phase = VM_MACHINE_HDC_PHASE_DATA_WRITE;
        hdc->data.sectors_remaining = hdc->data.sector_count == 0u ? 256u :
            hdc->data.sector_count;
        hdc->data.data_index = 0u;
        hdc->data.status = VM_MACHINE_HDC_STATUS_DRDY | VM_MACHINE_HDC_STATUS_DSC |
            VM_MACHINE_HDC_STATUS_DRQ;
        vm_machine_hdc_raise_irq(hdc);
        break;
    default:
        vm_machine_hdc_fail(hdc, VM_MACHINE_HDC_ERROR_ABORT);
        break;
    }
}

static type_status vm_machine_hdc_port_read(C_VOID *opaque, uint16_t port,
    uint32_t *out_value)
{
    vm_machine_hdc *hdc = (vm_machine_hdc *)opaque;
    uint16_t word;

    if (hdc == STD_NULL || out_value == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_value = 0u;
    if (port == hdc->connect.config.data_port) {
        if (hdc->data.phase != VM_MACHINE_HDC_PHASE_DATA_READ ||
            hdc->data.data_index >= sizeof(hdc->data.data)) {
            return TYPE_STATUS_OK;
        }
        STD_MEMCPY(&word, &hdc->data.data[hdc->data.data_index], sizeof(word));
        *out_value = word;
        hdc->data.data_index = (uint16_t)(hdc->data.data_index + sizeof(word));
        if (hdc->data.data_index == sizeof(hdc->data.data)) {
            vm_machine_hdc_next_read_sector(hdc);
        }
        return TYPE_STATUS_OK;
    }
    if (port == hdc->connect.config.error_features_port) {
        *out_value = hdc->data.error;
    } else if (port == hdc->connect.config.sector_count_port) {
        *out_value = hdc->data.sector_count;
    } else if (port == hdc->connect.config.sector_number_port) {
        *out_value = hdc->data.sector_number;
    } else if (port == hdc->connect.config.cylinder_low_port) {
        *out_value = hdc->data.cylinder_low;
    } else if (port == hdc->connect.config.cylinder_high_port) {
        *out_value = hdc->data.cylinder_high;
    } else if (port == hdc->connect.config.drive_head_port) {
        *out_value = hdc->data.drive_head;
    } else if (port == hdc->connect.config.status_command_port) {
        *out_value = hdc->data.status;
        vm_machine_hdc_clear_irq(hdc);
    } else if (port == hdc->connect.config.alternate_status_device_control_port) {
        *out_value = hdc->data.status;
    } else {
        return TYPE_STATUS_UNSUPPORTED;
    }
    return TYPE_STATUS_OK;
}

static type_status vm_machine_hdc_port_write(C_VOID *opaque, uint16_t port,
    uint32_t value)
{
    vm_machine_hdc *hdc = (vm_machine_hdc *)opaque;

    if (hdc == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (port == hdc->connect.config.data_port) {
        uint16_t word = (uint16_t)value;
        if (hdc->data.phase != VM_MACHINE_HDC_PHASE_DATA_WRITE ||
            hdc->data.data_index >= sizeof(hdc->data.data)) {
            return TYPE_STATUS_OK;
        }
        STD_MEMCPY(&hdc->data.data[hdc->data.data_index], &word, sizeof(word));
        hdc->data.data_index = (uint16_t)(hdc->data.data_index + sizeof(word));
        if (hdc->data.data_index == sizeof(hdc->data.data)) {
            vm_machine_hdc_next_write_sector(hdc);
        }
        return TYPE_STATUS_OK;
    }
    if (port == hdc->connect.config.error_features_port) {
        hdc->data.features = (uint8_t)value;
    } else if (port == hdc->connect.config.sector_count_port) {
        hdc->data.sector_count = (uint8_t)value;
    } else if (port == hdc->connect.config.sector_number_port) {
        hdc->data.sector_number = (uint8_t)value;
    } else if (port == hdc->connect.config.cylinder_low_port) {
        hdc->data.cylinder_low = (uint8_t)value;
    } else if (port == hdc->connect.config.cylinder_high_port) {
        hdc->data.cylinder_high = (uint8_t)value;
    } else if (port == hdc->connect.config.drive_head_port) {
        hdc->data.drive_head = (uint8_t)value;
    } else if (port == hdc->connect.config.status_command_port) {
        vm_machine_hdc_execute_command(hdc, (uint8_t)value);
    } else if (port == hdc->connect.config.alternate_status_device_control_port) {
        uint8_t device_control = (uint8_t)value;
        type_bool reset_asserted = (device_control &
            VM_MACHINE_HDC_DEVICE_CONTROL_SRST) != 0u;

        hdc->data.device_control = device_control;
        if (reset_asserted && !hdc->data.reset_asserted) {
            hdc->data.reset_asserted = TYPE_TRUE;
            hdc->data.phase = VM_MACHINE_HDC_PHASE_IDLE;
            hdc->data.data_index = 0u;
            hdc->data.sectors_remaining = 0u;
            hdc->data.error = 0u;
            hdc->data.status = VM_MACHINE_HDC_STATUS_BSY;
            vm_machine_hdc_clear_irq(hdc);
        } else if (!reset_asserted && hdc->data.reset_asserted) {
            vm_machine_hdc_reset(hdc);
            hdc->data.device_control = device_control;
        }
    } else {
        return TYPE_STATUS_UNSUPPORTED;
    }
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider vm_machine_hdc_ports = {
    vm_machine_hdc_port_read,
    vm_machine_hdc_port_write
};

C_VOID vm_machine_hdc_connect(vm_machine_hdc *hdc,
    const core_machine_media_registry *media_registry,
    core_machine_media_id media_id,
    t_pic *pic_master, t_pic *pic_slave, const vm_machine_hdc_config *config)
{
    if (hdc == STD_NULL || config == STD_NULL) return;
    hdc->connect.media_registry = media_registry;
    hdc->connect.media_id = media_id;
    core_machine_pic_irq_source_bind(&hdc->connect.irq_source, pic_master,
        pic_slave, config->irq);
    hdc->connect.config = *config;
}

C_VOID vm_machine_hdc_initialize(vm_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    STD_MEMSET(&hdc->data, 0, sizeof(hdc->data));
    core_machine_pic_irq_source_deassert(&hdc->connect.irq_source);
    vm_machine_hdc_reset(hdc);
}

C_VOID vm_machine_hdc_reset(vm_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    STD_MEMSET(&hdc->data, 0, sizeof(hdc->data));
    vm_machine_hdc_clear_irq(hdc);
    hdc->data.status = VM_MACHINE_HDC_STATUS_DRDY | VM_MACHINE_HDC_STATUS_DSC;
}

C_VOID vm_machine_hdc_refresh(vm_machine_hdc *hdc)
{
    (C_VOID)hdc;
}

C_VOID vm_machine_hdc_finalize(vm_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    STD_MEMSET(&hdc->data, 0, sizeof(hdc->data));
    STD_MEMSET(&hdc->connect, 0, sizeof(hdc->connect));
}

const core_machine_port_provider *vm_machine_hdc_port_provider(C_VOID)
{
    return &vm_machine_hdc_ports;
}

type_bool vm_machine_hdc_irq_pending(const vm_machine_hdc *hdc)
{
    return hdc != STD_NULL && hdc->data.irq_pending;
}
