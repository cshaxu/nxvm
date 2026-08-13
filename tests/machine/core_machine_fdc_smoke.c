#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/fdc.h"
#include "core/machine/machine.h"
#include "core/machine/media_interface.h"
#include "core/machine/port.h"

typedef struct core_machine_fdc_fixture_media {
    type_unsigned_8 bytes[512];
    type_unsigned_64 generation;
    type_unsigned_32 query_count;
    type_unsigned_32 read_count;
    type_unsigned_32 write_count;
    type_unsigned_32 format_count;
    type_bool present;
    type_bool read_only;
    core_machine_media_result forced_read_result;
    core_machine_media_result forced_write_result;
    core_machine_media_result forced_format_result;
} core_machine_fdc_fixture_media;

static core_machine_media_result core_machine_fdc_fixture_query(C_VOID *context,
    core_machine_media_info *out_info)
{
    core_machine_fdc_fixture_media *media = context;

    if (media == STD_NULL || out_info == STD_NULL) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->query_count;
    STD_MEMSET(out_info, 0, sizeof(*out_info));
    out_info->generation = media->generation;
    out_info->present = media->present;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_REMOVABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
        CORE_MACHINE_MEDIA_CAPABILITY_CHANGE_DETECTABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_FORMATTABLE;
    if (media->read_only) out_info->capabilities |= CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY;
    out_info->geometry.cylinders = 1u;
    out_info->geometry.heads = 1u;
    out_info->geometry.sectors_per_track = 1u;
    out_info->geometry.bytes_per_sector = 512u;
    out_info->geometry.logical_sector_count = 1u;
    return media->present ? CORE_MACHINE_MEDIA_RESULT_OK : CORE_MACHINE_MEDIA_RESULT_ABSENT;
}

static core_machine_media_result core_machine_fdc_fixture_read(C_VOID *context,
    type_unsigned_64 offset, C_VOID *buffer, type_unsigned_32 byte_count)
{
    core_machine_fdc_fixture_media *media = context;

    if (media == STD_NULL || buffer == STD_NULL || !media->present) {
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    }
    if (media->forced_read_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        return media->forced_read_result;
    }
    if (offset >= sizeof(media->bytes) || byte_count != 1u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->read_count;
    *(type_unsigned_8 *)buffer = media->bytes[offset];
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_fdc_fixture_write(C_VOID *context,
    type_unsigned_64 offset, const C_VOID *buffer, type_unsigned_32 byte_count)
{
    core_machine_fdc_fixture_media *media = context;

    if (media == STD_NULL || buffer == STD_NULL || !media->present) {
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    }
    if (media->read_only) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (media->forced_write_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        return media->forced_write_result;
    }
    if (offset >= sizeof(media->bytes) || byte_count != 1u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->write_count;
    media->bytes[offset] = *(const type_unsigned_8 *)buffer;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_fdc_fixture_format(C_VOID *context,
    type_unsigned_64 logical_sector, type_unsigned_32 sector_count, type_unsigned_8 fill)
{
    core_machine_fdc_fixture_media *media = context;

    if (media == STD_NULL || !media->present) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (media->read_only) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (media->forced_format_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        return media->forced_format_result;
    }
    if (logical_sector != 0u || sector_count != 1u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->format_count;
    ++media->generation;
    STD_MEMSET(media->bytes, fill, sizeof(media->bytes));
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider core_machine_fdc_fixture_provider = {
    core_machine_fdc_fixture_query,
    core_machine_fdc_fixture_read,
    core_machine_fdc_fixture_write,
    core_machine_fdc_fixture_format,
    STD_NULL
};

static C_VOID core_machine_fdc_command(core_machine_fdc *fdc, t_port *port,
    const type_unsigned_8 *bytes, STD_SIZE_T count)
{
    STD_SIZE_T index;

    for (index = 0u; index < count; ++index) {
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    }
    core_machine_fdc_advance(fdc);
}

static C_INT core_machine_fdc_read_result(core_machine_fdc *fdc, t_port *port,
    type_unsigned_8 *result, STD_SIZE_T count)
{
    STD_SIZE_T index;

    core_machine_fdc_advance(fdc);
    for (index = 0u; index < count; ++index) {
        result[index] = (type_unsigned_8)core_machine_port_read(port, 0x03f5u);
    }
    return (core_machine_port_read(port, 0x03f4u) & (VFDC_MSR_CB | VFDC_MSR_DIO)) == 0u;
}

C_INT main(C_VOID)
{
    static const type_unsigned_8 specify_non_dma[] = {0x03u, 0xdfu, 0x03u};
    static const type_unsigned_8 read_sector[] = {
        0xe6u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const type_unsigned_8 write_sector[] = {
        0xc5u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const type_unsigned_8 format_track[] = {
        0x4du, 0x00u, 0x02u, 0x01u, 0x1bu, 0xa5u
    };
    static const type_unsigned_8 format_id[] = {0x00u, 0x00u, 0x01u, 0x02u};
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_fdc_config fdc_config = {
        .dor_port = 0x03f2u, .status_port = 0x03f4u, .data_port = 0x03f5u,
        .direction_port = 0x03f7u, .control_port = 0x03f7u,
        .irq = 6u, .dma_channel = 2u
    };
    const core_machine_fdc_drive_bindings drives = {
        {1u, CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID}
    };
    const core_machine_dma_wiring dma_wiring = {.fdc_channel = 2u};
    core_machine_fdc_fixture_media fixture = {
        .generation = 1u, .present = TYPE_TRUE,
        .forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK,
        .forced_write_result = CORE_MACHINE_MEDIA_RESULT_OK,
        .forced_format_result = CORE_MACHINE_MEDIA_RESULT_OK
    };
    core_machine_media_registry media = {0};
    core_machine_dma_request_binding dma_request = {0};
    core_machine_fdc_topology topology = {0};
    core_machine *machine = STD_NULL;
    core_machine_fdc *fdc;
    t_port *port;
    type_unsigned_8 result[7];
    C_INT failed = 0;

    fixture.bytes[0] = 0x4au;
    core_machine_media_registry_initialize(&media);
    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK) failed |= 0x01;
    if (!failed) {
        fdc = &machine->fdc;
        port = &machine->executor_port;
        if (fdc == STD_NULL || port == STD_NULL ||
            core_machine_media_registry_bind(&media, 1u, &fixture,
                &core_machine_fdc_fixture_provider) != TYPE_STATUS_OK ||
            core_machine_media_registry_freeze(&media) != TYPE_STATUS_OK ||
            core_machine_media_registry_bind(&media, 2u, &fixture,
                &core_machine_fdc_fixture_provider) != TYPE_STATUS_INVALID_STATE ||
            core_machine_configure_dma(machine, &dma_wiring, &dma_request) !=
                TYPE_STATUS_OK) {
            failed |= 0x02;
        } else {
            topology.media_registry = &media;
            topology.drives = drives;
            topology.dma_request = dma_request;
            topology.config = fdc_config;
            if (core_machine_configure_fdc(machine, &topology) != TYPE_STATUS_OK ||
                core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
                core_machine_reset(machine) != TYPE_STATUS_OK) {
                failed |= 0x04;
            } else {
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));

                for (type_unsigned_32 index = 0u; index < sizeof(read_sector); ++index) {
                    core_machine_port_write(port, fdc_config.data_port, read_sector[index]);
                }
                failed |= fdc->data.phase != core_machine_fdc_PHASE_PENDING_COMMAND ||
                    core_machine_port_read(port, fdc_config.status_port) != VFDC_MSR_CB ||
                    fdc->connect.irq_source.asserted;
                core_machine_fdc_advance(fdc);
                failed |= fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_READ;
                failed |= core_machine_port_read(port, fdc_config.data_port) != 0x4au;
                for (type_unsigned_32 index = 1u; index < 512u; ++index) {
                    (C_VOID)core_machine_port_read(port, fdc_config.data_port);
                }
                failed |= fdc->data.phase != core_machine_fdc_PHASE_PENDING_COMPLETE ||
                    core_machine_port_read(port, fdc_config.status_port) != VFDC_MSR_CB ||
                    fdc->connect.irq_source.asserted;
                core_machine_fdc_advance(fdc);
                failed |= fdc->data.phase != core_machine_fdc_PHASE_RESULT ||
                    !fdc->connect.irq_source.asserted ||
                    !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != core_machine_fdc_ST0_NORMAL;
                fixture.read_count = 0u;

                core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                failed |= core_machine_port_read(port, fdc_config.data_port) != 0x4au;
                for (type_unsigned_32 index = 1u; index < 512u; ++index) {
                    (C_VOID)core_machine_port_read(port, fdc_config.data_port);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != core_machine_fdc_ST0_NORMAL || fixture.read_count != 512u;

                core_machine_fdc_command(fdc, port, write_sector, sizeof(write_sector));
                for (type_unsigned_32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x5au : 0u);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != core_machine_fdc_ST0_NORMAL || fixture.write_count != 512u ||
                    fixture.bytes[0] != 0x5au;

                core_machine_fdc_command(fdc, port, format_track, sizeof(format_track));
                core_machine_fdc_command(fdc, port, format_id, sizeof(format_id));
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != core_machine_fdc_ST0_NORMAL || fixture.format_count != 1u ||
                    fixture.generation != 2u || fixture.bytes[511] != 0xa5u;

                core_machine_fdc_command(fdc, port,
                    (const type_unsigned_8[]){0x0fu, 0x00u, 0x00u}, 3u);
                core_machine_fdc_command(fdc, port, (const type_unsigned_8[]){0x08u}, 1u);
                failed |= !core_machine_fdc_read_result(fdc, port, result, 2u);
                core_machine_fdc_refresh(fdc);
                failed |= (core_machine_port_read(port, fdc_config.direction_port) & VFDC_DIR_DC) != 0u;
                ++fixture.generation;
                core_machine_fdc_refresh(fdc);
                failed |= (core_machine_port_read(port, fdc_config.direction_port) & VFDC_DIR_DC) == 0u;

                fixture.forced_read_result = CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
                core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                (C_VOID)core_machine_port_read(port, fdc_config.data_port);
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[1] & 0x04u) == 0u;
                fixture.forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK;
                fixture.read_only = TYPE_TRUE;
                core_machine_fdc_command(fdc, port, write_sector, sizeof(write_sector));
                core_machine_port_write(port, fdc_config.data_port, 0x33u);
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[1] & 0x02u) == 0u;
                fixture.read_only = TYPE_FALSE;
                fixture.present = TYPE_FALSE;
                core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[1] & 0x04u) == 0u;
            }
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_finalize(&media);
    if (failed) return 1;
    puts("M5:T283:S2:CORE-FDC-MEDIA:OK");
    puts("M5:T347:S2:FDC-SERVICE:OK");
    return 0;
}
