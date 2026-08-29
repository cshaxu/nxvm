#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/machine.h"
#include "core/machine/media_interface.h"
#include "core/machine/port.h"

typedef struct core_machine_compaq_hdc_machine_fdc_media {
    type_unsigned_8 byte;
} core_machine_compaq_hdc_machine_fdc_media;

static core_machine_media_result core_machine_compaq_hdc_machine_fdc_query(C_VOID *opaque,
    core_machine_media_info *out_info)
{
    if (opaque == STD_NULL || out_info == STD_NULL) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    STD_MEMSET(out_info, 0, sizeof(*out_info));
    out_info->present = TYPE_TRUE;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_REMOVABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN;
    out_info->geometry.cylinders = 1u;
    out_info->geometry.heads = 1u;
    out_info->geometry.sectors_per_track = 1u;
    out_info->geometry.bytes_per_sector = 512u;
    out_info->geometry.logical_sector_count = 1u;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_compaq_hdc_machine_fdc_read(C_VOID *opaque,
    type_unsigned_64 offset, C_VOID *buffer, type_unsigned_32 byte_count)
{
    core_machine_compaq_hdc_machine_fdc_media *media = opaque;

    if (media == STD_NULL || buffer == STD_NULL || offset >= 512u || byte_count != 1u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    *(type_unsigned_8 *)buffer = media->byte;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider core_machine_compaq_hdc_machine_fdc_provider = {
    core_machine_compaq_hdc_machine_fdc_query,
    core_machine_compaq_hdc_machine_fdc_read,
    STD_NULL,
    STD_NULL,
    STD_NULL,
    STD_NULL,
    STD_NULL
};

static core_machine_media_result core_machine_compaq_hdc_machine_hdc_query(C_VOID *opaque,
    core_machine_media_info *out_info)
{
    if (opaque == STD_NULL || out_info == STD_NULL) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    STD_MEMSET(out_info, 0, sizeof(*out_info));
    out_info->present = TYPE_TRUE;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN;
    out_info->geometry.cylinders = 1u;
    out_info->geometry.heads = 16u;
    out_info->geometry.sectors_per_track = 17u;
    out_info->geometry.bytes_per_sector = 512u;
    out_info->geometry.logical_sector_count = 272u;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_compaq_hdc_machine_hdc_read(C_VOID *opaque,
    type_unsigned_64 offset, C_VOID *buffer, type_unsigned_32 byte_count)
{
    type_unsigned_8 *sector = opaque;

    if (sector == STD_NULL || buffer == STD_NULL || offset != 0u || byte_count != 512u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    STD_MEMCPY(buffer, sector, byte_count);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider core_machine_compaq_hdc_machine_hdc_provider = {
    core_machine_compaq_hdc_machine_hdc_query,
    core_machine_compaq_hdc_machine_hdc_read,
    STD_NULL,
    STD_NULL,
    STD_NULL,
    STD_NULL,
    STD_NULL
};

C_INT main(C_VOID)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_dma_wiring dma_wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    const core_machine_fdc_config fdc_config = {
        .dor_port = 0x03f2u, .status_port = 0x03f4u, .data_port = 0x03f5u,
        .direction_port = 0x03f7u, .control_port = 0x03f7u,
        .irq = 6u, .dma_channel = 2u
    };
    const core_machine_fdc_drive_bindings drives = {
        {11u, CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID}, 0x01u, 0x01u, {0u, 0u, 0u, 0u}, 0u
    };
    const core_machine_hdc_config hdc_config = {
        .protocol = CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB, .irq = 14u,
        .bus.task_file = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .drive_address_port = 0x03f7u, .lba28_supported = TYPE_FALSE}
    };
    core_machine_compaq_hdc_machine_fdc_media fdc_media = {.byte = 0x5au};
    type_unsigned_8 hdc_sector[512] = {0};
    core_machine_media_registry *media = STD_NULL;
    core_machine_dma_request_binding dma_request = {0};
    core_machine_fdc_topology fdc_topology = {0};
    core_machine_hdc_topology hdc_topology = {0};
    core_machine *machine = STD_NULL;
    type_unsigned_32 drive_address;
    C_INT failed = 0;

    if (core_machine_media_registry_create(&media) != TYPE_STATUS_OK ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(media, 11u, &fdc_media,
            &core_machine_compaq_hdc_machine_fdc_provider) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(media, 12u, hdc_sector,
            &core_machine_compaq_hdc_machine_hdc_provider) != TYPE_STATUS_OK ||
        core_machine_media_registry_freeze(media) != TYPE_STATUS_OK ||
        core_machine_configure_dma(machine, &dma_wiring, &dma_request) != TYPE_STATUS_OK) {
        failed = 0x01;
    } else {
        fdc_topology.media_registry = media;
        fdc_topology.drives = drives;
        fdc_topology.dma_request = dma_request;
        fdc_topology.config = fdc_config;
        hdc_topology.media_registry = media;
        hdc_topology.media_id = 12u;
        hdc_topology.config = hdc_config;
        if (core_machine_configure_fdc(machine, &fdc_topology) != TYPE_STATUS_OK ||
            core_machine_configure_hdc(machine, &hdc_topology) != TYPE_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
            core_machine_reset(machine) != TYPE_STATUS_OK) {
            failed = 0x02;
        } else {
            core_machine_port_write(&machine->executor_port, 0x01f6u, 0x2au);
            drive_address = core_machine_port_read(&machine->executor_port, 0x03f7u);
            failed = (drive_address & 0x1fu) != 0x0au ? 0x04 : 0;
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    if (failed) {
        STD_FPRINTF(stderr, "M5:T386:S5:COMPAQ-HDC-MACHINE:FAIL:%x\n", failed);
        return 1;
    }
    puts("M5:T386:S5:COMPAQ-HDC-MACHINE:OK");
    return 0;
}
