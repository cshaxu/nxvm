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
    core_machine_media_address_mark mark;
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
        CORE_MACHINE_MEDIA_CAPABILITY_FORMATTABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_ADDRESS_MARKS;
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

static core_machine_media_result core_machine_fdc_fixture_get_mark(C_VOID *context,
    type_unsigned_64 logical_sector, core_machine_media_address_mark *out_mark)
{
    core_machine_fdc_fixture_media *media = context;

    if (media == STD_NULL || out_mark == STD_NULL || !media->present)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (logical_sector != 0u) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    *out_mark = media->mark;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_fdc_fixture_set_mark(C_VOID *context,
    type_unsigned_64 logical_sector, core_machine_media_address_mark mark)
{
    core_machine_fdc_fixture_media *media = context;

    if (media == STD_NULL || !media->present) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (media->read_only) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (logical_sector != 0u || (mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA &&
        mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA))
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    media->mark = mark;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider core_machine_fdc_fixture_provider = {
    core_machine_fdc_fixture_query,
    core_machine_fdc_fixture_read,
    core_machine_fdc_fixture_write,
    core_machine_fdc_fixture_format,
    STD_NULL,
    core_machine_fdc_fixture_get_mark,
    core_machine_fdc_fixture_set_mark
};

static C_VOID core_machine_fdc_command(core_machine_fdc *fdc, t_port *port,
    const type_unsigned_8 *bytes, STD_SIZE_T count)
{
    STD_SIZE_T index;

    for (index = 0u; index < count; ++index) {
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    }
    core_machine_fdc_advance(fdc);
    for (type_unsigned_8 drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        if (fdc->data.seek_pending[drive]) {
            core_machine_fdc_advance_at(fdc, fdc->data.seek_due_tick[drive]);
        }
    }
}

static C_VOID core_machine_fdc_write_dma2(t_port *port, type_unsigned_16 address,
    type_unsigned_16 count)
{
    core_machine_port_write(port, 0x000cu, 0u);
    core_machine_port_write(port, 0x0004u, address & 0xffu);
    core_machine_port_write(port, 0x0004u, address >> 8u);
    core_machine_port_write(port, 0x0005u, count & 0xffu);
    core_machine_port_write(port, 0x0005u, count >> 8u);
    core_machine_port_write(port, 0x0081u, 0u);
    core_machine_port_write(port, 0x000bu, 0x46u);
    core_machine_port_write(port, 0x000au, 0x02u);
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
    static const type_unsigned_8 read_deleted_sector[] = {
        0xecu, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const type_unsigned_8 write_deleted_sector[] = {
        0xc9u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const type_unsigned_8 scan_equal[] = {
        0x11u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const type_unsigned_8 scan_low_or_equal[] = {
        0x19u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const type_unsigned_8 scan_high_or_equal[] = {
        0x1du, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const type_unsigned_8 scan_equal_skip[] = {
        0x31u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
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
        .irq = 6u, .dma_channel = 2u, .ticks_per_microsecond = 8u
    };
    const core_machine_fdc_drive_bindings drives = {
        {1u, CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID}
    };
    const core_machine_dma_wiring dma_wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_fdc_fixture_media fixture = {
        .generation = 1u, .present = TYPE_TRUE,
        .forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK,
        .forced_write_result = CORE_MACHINE_MEDIA_RESULT_OK,
        .forced_format_result = CORE_MACHINE_MEDIA_RESULT_OK
    };
    core_machine_media_registry *media = STD_NULL;
    core_machine_dma_request_binding dma_request = {0};
    core_machine_fdc_topology topology = {0};
    core_machine *machine = STD_NULL;
    core_machine_fdc *fdc;
    t_port *port;
    type_unsigned_8 result[7];
    type_unsigned_8 scan_dma[512];
    type_unsigned_64 ndma_gate_tick;
    C_INT failed = 0;

    fixture.bytes[0] = 0x4au;
    if (core_machine_media_registry_create(&media) != TYPE_STATUS_OK ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK) failed |= 0x01;
    if (!failed) {
        fdc = &machine->fdc;
        port = &machine->executor_port;
        if (fdc == STD_NULL || port == STD_NULL ||
            core_machine_media_registry_bind(media, 1u, &fixture,
                &core_machine_fdc_fixture_provider) != TYPE_STATUS_OK ||
            core_machine_media_registry_freeze(media) != TYPE_STATUS_OK ||
            core_machine_media_registry_bind(media, 2u, &fixture,
                &core_machine_fdc_fixture_provider) != TYPE_STATUS_INVALID_STATE ||
            core_machine_configure_dma(machine, &dma_wiring, &dma_request) !=
                TYPE_STATUS_OK) {
            failed |= 0x02;
        } else {
            topology.media_registry = media;
            topology.drives = drives;
            topology.dma_request = dma_request;
            topology.config = fdc_config;
            if (core_machine_configure_fdc(machine, &topology) != TYPE_STATUS_OK ||
                core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
                core_machine_reset(machine) != TYPE_STATUS_OK) {
                failed |= 0x04;
            } else {
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                failed |= fdc->connect.irq_source.asserted ||
                    !fdc->data.reset_pending || fdc->data.reset_due_tick != 8192u;
                core_machine_fdc_advance_at(fdc, 8191u);
                failed |= fdc->connect.irq_source.asserted;
                core_machine_fdc_advance_at(fdc, 8192u);
                failed |= !fdc->connect.irq_source.asserted;
                for (type_unsigned_8 reset_drive = 0u;
                    reset_drive < 1u; ++reset_drive) {
                    core_machine_fdc_command(fdc, port,
                        (const type_unsigned_8[]){0x08u}, 1u);
                    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                        result[0] != (core_machine_fdc_ST0_READY_CHANGE | reset_drive) ||
                        result[1] != 0u || fdc->connect.irq_source.asserted;
                }
                core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));
                core_machine_port_write(port, fdc_config.control_port, VFDC_CCR_DRC);

                core_machine_fdc_command(fdc, port, (const type_unsigned_8[]){0x10u}, 1u);
                failed |= fdc->connect.irq_source.asserted ||
                    !core_machine_fdc_read_result(fdc, port, result, 1u) ||
                    result[0] != 0x80u || fdc->data.phase != core_machine_fdc_PHASE_COMMAND;

                /* The rendered uPD765 reset record preserves Specify's SRT,
                   HUT and HLT fields across either DOR reset edge. */
                failed |= fdc->data.srt != 0x0du || fdc->data.hut != 0x0fu ||
                    fdc->data.hlt != 0x01u;
                core_machine_port_write(port, fdc_config.dor_port, 0x18u);
                failed |= fdc->data.srt != 0x0du || fdc->data.hut != 0x0fu ||
                    fdc->data.hlt != 0x01u;
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_advance_at(fdc, fdc->data.reset_due_tick);
                failed |= fdc->data.srt != 0x0du || fdc->data.hut != 0x0fu ||
                    fdc->data.hlt != 0x01u;
                for (type_unsigned_8 reset_drive = 0u;
                    reset_drive < 1u; ++reset_drive) {
                    core_machine_fdc_command(fdc, port,
                        (const type_unsigned_8[]){0x08u}, 1u);
                    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                        result[0] != (core_machine_fdc_ST0_READY_CHANGE | reset_drive) ||
                        result[1] != 0u || fdc->connect.irq_source.asserted;
                }
                core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));

                /* Seek must retain the prior cylinder and IRQ state until the
                   source-labelled 3-ms-per-track deadline has elapsed. */
                core_machine_port_write(port, fdc_config.data_port, 0x0fu);
                core_machine_port_write(port, fdc_config.data_port, 0x00u);
                core_machine_port_write(port, fdc_config.data_port, 0x03u);
                core_machine_fdc_advance_at(fdc, 100u);
                failed |= fdc->data.seek_pending[0u] == TYPE_FALSE ||
                    fdc->data.cylinder != 0u || fdc->connect.irq_source.asserted ||
                    fdc->data.seek_due_tick[0u] != 72100u;
                core_machine_fdc_advance_at(fdc, 72099u);
                failed |= fdc->data.seek_pending[0u] == TYPE_FALSE ||
                    fdc->connect.irq_source.asserted;
                core_machine_fdc_advance_at(fdc, 72100u);
                failed |= fdc->data.cylinder != 3u || !fdc->connect.irq_source.asserted;
                core_machine_fdc_command(fdc, port, (const type_unsigned_8[]){0x08u}, 1u);
                failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                    result[1] != 3u;

                /* Intel 8272A permits one drive to seek while another is
                   commanded.  Each completion must remain associated with
                   its own drive until Sense Interrupt Status consumes it. */
                core_machine_port_write(port, fdc_config.data_port, 0x0fu);
                core_machine_port_write(port, fdc_config.data_port, 0x00u);
                core_machine_port_write(port, fdc_config.data_port, 0x07u);
                core_machine_fdc_advance_at(fdc, 72101u);
                core_machine_port_write(port, fdc_config.data_port, 0x0fu);
                core_machine_port_write(port, fdc_config.data_port, 0x01u);
                core_machine_port_write(port, fdc_config.data_port, 0x01u);
                core_machine_fdc_advance_at(fdc, 72102u);
                failed |= !fdc->data.seek_pending[0u] || !fdc->data.seek_pending[1u] ||
                    (core_machine_port_read(port, fdc_config.status_port) &
                    (VFDC_MSR_DB(0u) | VFDC_MSR_DB(1u))) !=
                    (VFDC_MSR_DB(0u) | VFDC_MSR_DB(1u));
                core_machine_fdc_advance_at(fdc, 96102u);
                failed |= fdc->data.seek_pending[1u] || !fdc->data.seek_pending[0u] ||
                    !fdc->connect.irq_source.asserted;
                core_machine_port_write(port, fdc_config.data_port, 0x08u);
                core_machine_fdc_advance_at(fdc, 96103u);
                failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                    (result[0] & 3u) != 1u || result[1] != 1u;
                core_machine_fdc_advance_at(fdc, 168101u);
                core_machine_port_write(port, fdc_config.data_port, 0x08u);
                core_machine_fdc_advance_at(fdc, 168102u);
                failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                    (result[0] & 3u) != 0u || result[1] != 7u;

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

                fixture.mark = CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA;
                core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                for (type_unsigned_32 index = 0u; index < 512u; ++index) {
                    (C_VOID)core_machine_port_read(port, fdc_config.data_port);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & VFDC_ST2_CONTROL_MARK) == 0u;
                core_machine_fdc_command(fdc, port, read_deleted_sector,
                    sizeof(read_deleted_sector));
                for (type_unsigned_32 index = 0u; index < 512u; ++index) {
                    (C_VOID)core_machine_port_read(port, fdc_config.data_port);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & VFDC_ST2_CONTROL_MARK) != 0u;

                fixture.mark = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
                core_machine_fdc_command(fdc, port, write_deleted_sector,
                    sizeof(write_deleted_sector));
                for (type_unsigned_32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x6bu : 0u);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != core_machine_fdc_ST0_NORMAL ||
                    fixture.mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA ||
                    fixture.bytes[0] != 0x6bu;

                fixture.write_count = 0u;
                core_machine_fdc_command(fdc, port, write_sector, sizeof(write_sector));
                for (type_unsigned_32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x5au : 0u);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != core_machine_fdc_ST0_NORMAL || fixture.write_count != 512u ||
                    fixture.bytes[0] != 0x5au;

                /* Scan commands receive comparison bytes through the same
                   host-to-controller path as a write, but never mutate media.
                   FFh is the documented no-care compare byte. */
                fixture.mark = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
                core_machine_fdc_command(fdc, port, scan_equal, sizeof(scan_equal));
                for (type_unsigned_32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x5au : 0xffu);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (VFDC_ST2_SCAN_MATCH | VFDC_ST2_SCAN_MISMATCH)) !=
                        VFDC_ST2_SCAN_MATCH || fixture.bytes[0] != 0x5au;
                core_machine_fdc_command(fdc, port, scan_low_or_equal,
                    sizeof(scan_low_or_equal));
                for (type_unsigned_32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x50u : 0xffu);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (VFDC_ST2_SCAN_MATCH | VFDC_ST2_SCAN_MISMATCH)) !=
                        VFDC_ST2_SCAN_MATCH;
                core_machine_fdc_command(fdc, port, scan_high_or_equal,
                    sizeof(scan_high_or_equal));
                for (type_unsigned_32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x60u : 0xffu);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (VFDC_ST2_SCAN_MATCH | VFDC_ST2_SCAN_MISMATCH)) !=
                        VFDC_ST2_SCAN_MATCH;
                core_machine_fdc_command(fdc, port, scan_equal, sizeof(scan_equal));
                for (type_unsigned_32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x50u : 0xffu);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (VFDC_ST2_SCAN_MATCH | VFDC_ST2_SCAN_MISMATCH)) !=
                        VFDC_ST2_SCAN_MISMATCH;
                fixture.mark = CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA;
                core_machine_fdc_command(fdc, port, scan_equal, sizeof(scan_equal));
                for (type_unsigned_32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x5au : 0xffu);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (VFDC_ST2_SCAN_MATCH | VFDC_ST2_CONTROL_MARK)) !=
                        (VFDC_ST2_SCAN_MATCH | VFDC_ST2_CONTROL_MARK);
                core_machine_fdc_command(fdc, port, scan_equal_skip,
                    sizeof(scan_equal_skip));
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (VFDC_ST2_SCAN_MATCH | VFDC_ST2_SCAN_MISMATCH |
                    VFDC_ST2_CONTROL_MARK)) != VFDC_ST2_SCAN_MISMATCH;
                fixture.mark = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;

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

                /* A 500-kbit/s DMA transfer exposes one byte, withdraws DRQ,
                   and cannot expose the next byte before its 128-tick gate. */
                fixture.read_count = 0u;
                core_machine_port_write(port, fdc_config.control_port, 0u);
                core_machine_fdc_command(fdc, port,
                    (const type_unsigned_8[]){0x03u, 0xdfu, 0x02u}, 3u);
                core_machine_fdc_write_dma2(port, 0x0600u, 1u);
                core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                core_machine_fdc_advance_at(fdc, 100u);
                failed |= !core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                    &machine->shared_dma_secondary);
                core_machine_dma_advance(&machine->shared_dma_latch,
                    &machine->shared_dma_primary, &machine->shared_dma_secondary,
                    &machine->executor_memory, 1u);
                failed |= fixture.read_count != 1u ||
                    core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                        &machine->shared_dma_secondary);
                core_machine_fdc_advance_at(fdc, 227u);
                failed |= core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                    &machine->shared_dma_secondary);
                core_machine_fdc_advance_at(fdc, 228u);
                failed |= !core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                    &machine->shared_dma_secondary);
                core_machine_dma_advance(&machine->shared_dma_latch,
                    &machine->shared_dma_primary, &machine->shared_dma_secondary,
                    &machine->executor_memory, 1u);
                failed |= fixture.read_count != 2u || fdc->data.phase !=
                    core_machine_fdc_PHASE_PENDING_COMPLETE || fdc->connect.irq_source.asserted;
                core_machine_fdc_advance_at(fdc, 229u);
                failed |= !fdc->connect.irq_source.asserted;
                core_machine_port_write(port, fdc_config.dor_port, 0u);
                failed |= core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                    &machine->shared_dma_secondary) || fdc->data.dma_byte_gate_pending;

                /* Scan consumes guest comparison bytes through DMA2's
                   memory-to-device direction, then reports through the same
                   seven-byte IRQ result phase. */
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_command(fdc, port,
                    (const type_unsigned_8[]){0x03u, 0xdfu, 0x02u}, 3u);
                core_machine_port_write(port, fdc_config.control_port, 0u);
                STD_MEMSET(scan_dma, 0xa5u, sizeof(scan_dma));
                failed |= core_machine_memory_write_physical(&machine->executor_memory,
                    0x0600u, (type_virtual_address)scan_dma, sizeof(scan_dma)) != TYPE_STATUS_OK;
                core_machine_fdc_write_dma2(port, 0x0600u, 511u);
                core_machine_port_write(port, 0x000bu, 0x4au);
                core_machine_fdc_command(fdc, port, scan_equal, sizeof(scan_equal));
                for (type_unsigned_32 index = 0u; index < sizeof(scan_dma); ++index) {
                    core_machine_dma_advance(&machine->shared_dma_latch,
                        &machine->shared_dma_primary, &machine->shared_dma_secondary,
                        &machine->executor_memory, 1u);
                    if (index + 1u < sizeof(scan_dma)) {
                        core_machine_fdc_advance_at(fdc,
                        fdc->data.elapsed_ticks + fdc_config.ticks_per_microsecond * 16u);
                    }
                }
                failed |= fdc->data.phase != core_machine_fdc_PHASE_PENDING_COMPLETE ||
                    fdc->data.dma_byte_gate_pending;
                core_machine_fdc_advance(fdc);
                failed |= !fdc->connect.irq_source.asserted ||
                    !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (VFDC_ST2_SCAN_MATCH | VFDC_ST2_SCAN_MISMATCH)) !=
                        VFDC_ST2_SCAN_MATCH;
                core_machine_port_write(port, fdc_config.dor_port, 0u);
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_advance_at(fdc, fdc->data.reset_due_tick);
                for (type_unsigned_8 reset_drive = 0u;
                    reset_drive < 1u; ++reset_drive) {
                    core_machine_fdc_command(fdc, port,
                        (const type_unsigned_8[]){0x08u}, 1u);
                    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                        result[0] != (core_machine_fdc_ST0_READY_CHANGE | reset_drive) ||
                        result[1] != 0u || fdc->connect.irq_source.asserted;
                }
                core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));
                core_machine_port_write(port, fdc_config.control_port, VFDC_CCR_DRC);

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

                /* The same 500-kbit/s byte interval applies when the host
                   services 3F5h directly rather than through DMA2. */
                fixture.present = TYPE_TRUE;
                fixture.read_count = 0u;
                core_machine_port_write(port, fdc_config.control_port, 0u);
                core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));
                core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                core_machine_fdc_advance_at(fdc, fdc->data.elapsed_ticks + 1u);
                failed |= core_machine_port_read(port, fdc_config.data_port) != 0xa5u ||
                    fixture.read_count != 1u || !fdc->data.ndma_byte_gate_pending;
                ndma_gate_tick = fdc->data.next_ndma_byte_tick;
                core_machine_fdc_advance_at(fdc, ndma_gate_tick - 1u);
                failed |= (core_machine_port_read(port, fdc_config.status_port) & VFDC_MSR_RQM) != 0u ||
                    fixture.read_count != 1u;
                core_machine_fdc_advance_at(fdc, ndma_gate_tick);
                failed |= (core_machine_port_read(port, fdc_config.status_port) &
                    VFDC_MSR_ProcessRead) != VFDC_MSR_ProcessRead ||
                    fdc->data.ndma_byte_gate_pending;
                (C_VOID)core_machine_port_read(port, fdc_config.data_port);
                failed |= fixture.read_count != 2u;
                core_machine_port_write(port, fdc_config.dor_port, 0u);
                failed |= fdc->data.ndma_byte_gate_pending;

                /* Scan is host-to-controller execution too: its first byte
                   establishes the same byte gate, and DOR reset cancels it. */
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_advance_at(fdc, fdc->data.reset_due_tick);
                for (type_unsigned_8 reset_drive = 0u;
                    reset_drive < 1u; ++reset_drive) {
                    core_machine_fdc_command(fdc, port,
                        (const type_unsigned_8[]){0x08u}, 1u);
                    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                        result[0] != (core_machine_fdc_ST0_READY_CHANGE | reset_drive) ||
                        result[1] != 0u || fdc->connect.irq_source.asserted;
                }
                core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));
                core_machine_port_write(port, fdc_config.control_port, 0u);
                core_machine_fdc_command(fdc, port, scan_equal, sizeof(scan_equal));
                core_machine_port_write(port, fdc_config.data_port, 0x5au);
                ndma_gate_tick = fdc->data.next_ndma_byte_tick;
                failed |= !fdc->data.ndma_byte_gate_pending ||
                    ndma_gate_tick != fdc->data.elapsed_ticks +
                    fdc_config.ticks_per_microsecond * 16u;
                core_machine_port_write(port, fdc_config.dor_port, 0u);
                failed |= fdc->data.phase != core_machine_fdc_PHASE_COMMAND ||
                    fdc->data.ndma_byte_gate_pending;
            }
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T376:S4:8272A-SCAN:FAIL %x\n", failed);
        return 1;
    }
    puts("M5:T283:S2:CORE-FDC-MEDIA:OK");
    puts("M5:T347:S2:FDC-SERVICE:OK");
    puts("M5:T375:S20:FDC-DMA-CADENCE:OK");
    puts("M5:T375:S21:FDC-SEEK-CADENCE:OK");
    puts("M5:T375:S24:FDC-NDMA-CADENCE:OK");
    puts("M5:T376:S3:8272A-DELETED-DATA:OK");
    puts("M5:T376:S4:8272A-SCAN:OK");
    puts("M5:T465:S2:FDC-reset:OK");
    puts("M5:T465:S3:FDC-8272-command:OK");
    puts("M5:T465:S5:FDC-parallel-seek:OK");
    return 0;
}
