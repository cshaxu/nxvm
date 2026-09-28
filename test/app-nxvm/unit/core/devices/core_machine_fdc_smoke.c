#include "support/dma_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/dma_bus.h"
#include "app-nxvm/devices/fdc.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/devices/port.h"

typedef struct core_machine_fdc_fixture_media {
    lib_u8 bytes[1536];
    lib_u8 sector_count;
    lib_u8 cylinder_count;
    lib_u8 head_count;
    lib_u64 generation;
    lib_u32 query_count;
    lib_u32 read_count;
    lib_u32 write_count;
    lib_u32 format_count;
    core_machine_media_address_mark marks[3];
    lib_u8 present;
    lib_u8 read_only;
    core_machine_media_result forced_read_result;
    core_machine_media_result forced_write_result;
    core_machine_media_result forced_format_result;
} core_machine_fdc_fixture_media;

static core_machine_media_result core_machine_fdc_fixture_query(void *context,
    core_machine_media_info *out_info)
{
    core_machine_fdc_fixture_media *media = context;

    if (media == LIB_NULL || out_info == LIB_NULL) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->query_count;
    lib_memory_set(out_info, 0, sizeof(*out_info));
    out_info->generation = media->generation;
    out_info->present = media->present;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_REMOVABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
        CORE_MACHINE_MEDIA_CAPABILITY_CHANGE_DETECTABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_FORMATTABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_ADDRESS_MARKS;
    if (media->read_only) out_info->capabilities |= CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY;
    out_info->geometry.cylinders = media->cylinder_count;
    out_info->geometry.heads = media->head_count;
    out_info->geometry.sectors_per_track = media->sector_count;
    out_info->geometry.bytes_per_sector = 512u;
    out_info->geometry.logical_sector_count = media->sector_count * media->cylinder_count * media->head_count;
    return media->present ? CORE_MACHINE_MEDIA_RESULT_OK : CORE_MACHINE_MEDIA_RESULT_ABSENT;
}

static core_machine_media_result core_machine_fdc_fixture_read(void *context,
    lib_u64 offset, void *buffer, lib_u32 byte_count)
{
    core_machine_fdc_fixture_media *media = context;

    if (media == LIB_NULL || buffer == LIB_NULL || !media->present) {
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    }
    if (media->forced_read_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        return media->forced_read_result;
    }
    if (offset >= (lib_u64)media->sector_count * media->cylinder_count * media->head_count * 512u ||
        offset >= sizeof(media->bytes) || byte_count != 1u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->read_count;
    *(lib_u8 *)buffer = media->bytes[offset];
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_fdc_fixture_write(void *context,
    lib_u64 offset, const void *buffer, lib_u32 byte_count)
{
    core_machine_fdc_fixture_media *media = context;

    if (media == LIB_NULL || buffer == LIB_NULL || !media->present) {
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    }
    if (media->read_only) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (media->forced_write_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        return media->forced_write_result;
    }
    if (offset >= (lib_u64)media->sector_count * media->cylinder_count * media->head_count * 512u ||
        offset >= sizeof(media->bytes) || byte_count != 1u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->write_count;
    media->bytes[offset] = *(const lib_u8 *)buffer;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_fdc_fixture_format(void *context,
    lib_u64 logical_sector, lib_u32 sector_count, lib_u8 fill)
{
    core_machine_fdc_fixture_media *media = context;

    if (media == LIB_NULL || !media->present) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (media->read_only) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (media->forced_format_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        return media->forced_format_result;
    }
    if (logical_sector >= (lib_u64)media->sector_count * media->cylinder_count * media->head_count ||
        logical_sector >= 3u || sector_count != 1u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->format_count;
    ++media->generation;
    lib_memory_set(media->bytes + logical_sector * 512u, fill, 512u);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_fdc_fixture_get_mark(void *context,
    lib_u64 logical_sector, core_machine_media_address_mark *out_mark)
{
    core_machine_fdc_fixture_media *media = context;

    if (media == LIB_NULL || out_mark == LIB_NULL || !media->present)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (logical_sector >= (lib_u64)media->sector_count * media->cylinder_count * media->head_count ||
        logical_sector >= 3u) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    *out_mark = media->marks[logical_sector];
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_fdc_fixture_set_mark(void *context,
    lib_u64 logical_sector, core_machine_media_address_mark mark)
{
    core_machine_fdc_fixture_media *media = context;

    if (media == LIB_NULL || !media->present) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (media->read_only) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (logical_sector >= (lib_u64)media->sector_count * media->cylinder_count * media->head_count ||
        logical_sector >= 3u || (mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA &&
        mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA))
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    media->marks[logical_sector] = mark;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider core_machine_fdc_fixture_provider = {
    core_machine_fdc_fixture_query,
    core_machine_fdc_fixture_read,
    core_machine_fdc_fixture_write,
    core_machine_fdc_fixture_format,
    LIB_NULL,
    core_machine_fdc_fixture_get_mark,
    core_machine_fdc_fixture_set_mark
};

static void core_machine_fdc_submit(core_machine_fdc *fdc, t_port *port,
    const lib_u8 *bytes, lib_size count)
{
    lib_size index;

    for (index = 0u; index < count; ++index) {
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    }
    core_machine_fdc_advance(fdc);
}

static void core_machine_fdc_finish_seeks(core_machine_fdc *fdc)
{
    for (lib_u16 steps = 0u; steps < 1024u; ++steps) {
        lib_u64 due = UINT64_MAX;
        for (lib_u8 drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive)
            if (fdc->data.seek_pending[drive] && fdc->data.seek_due_tick[drive] < due)
                due = fdc->data.seek_due_tick[drive];
        if (due == UINT64_MAX) return;
        core_machine_fdc_advance_at(fdc, due);
    }
}

static void core_machine_fdc_command(core_machine_fdc *fdc, t_port *port,
    const lib_u8 *bytes, lib_size count)
{
    core_machine_fdc_submit(fdc, port, bytes, count);
    core_machine_fdc_finish_seeks(fdc);
}

static void core_machine_fdc_write_dma2(t_port *port, lib_u16 address,
    lib_u16 count)
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

static lib_i32 core_machine_fdc_read_result(core_machine_fdc *fdc, t_port *port,
    lib_u8 *result, lib_size count)
{
    lib_size index;

    core_machine_fdc_advance(fdc);
    for (index = 0u; index < count; ++index) {
        if ((core_machine_port_read(port, 0x03f4u) & VFDC_MSR_ReadyRead) !=
            VFDC_MSR_ReadyRead) return LIB_FALSE;
        result[index] = (lib_u8)core_machine_port_read(port, 0x03f5u);
    }
    return (core_machine_port_read(port, 0x03f4u) & (VFDC_MSR_CB | VFDC_MSR_DIO)) == 0u;
}

/* S12 supersedes the S10 characterization: READY is a pin, whereas absent
 * media, motor and selection prevent a record from being delivered. */
static lib_i32 core_machine_fdc_readiness_matrix(core_machine *machine,
    core_machine_fdc_fixture_media *media)
{
    core_machine_fdc *fdc = &machine->fdc;
    t_port *port = &machine->executor_port;
    static const struct {
        lib_u8 bytes[9];
        lib_u8 count;
        core_machine_fdc_phase phase;
    } commands[] = {
        {{0x46u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu}, 9u,
            core_machine_fdc_PHASE_EXECUTION_READ},
        {{0x4cu, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu}, 9u,
            core_machine_fdc_PHASE_EXECUTION_READ},
        {{0x45u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu}, 9u,
            core_machine_fdc_PHASE_EXECUTION_WRITE},
        {{0x49u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu}, 9u,
            core_machine_fdc_PHASE_EXECUTION_WRITE},
        {{0x51u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 1u}, 9u,
            core_machine_fdc_PHASE_EXECUTION_SCAN},
        {{0x59u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 1u}, 9u,
            core_machine_fdc_PHASE_EXECUTION_SCAN},
        {{0x5du, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 1u}, 9u,
            core_machine_fdc_PHASE_EXECUTION_SCAN},
        {{0x42u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu}, 9u,
            core_machine_fdc_PHASE_EXECUTION_READ},
        {{0x4du, 0u, 2u, 1u, 0x1bu, 0xa5u}, 6u,
            core_machine_fdc_PHASE_EXECUTION_FORMAT},
        {{0x4au, 0u}, 2u, core_machine_fdc_PHASE_RESULT}
    };
    static const struct {
        lib_bool present;
        lib_u8 dor;
        lib_u8 ready;
        lib_u8 installed;
        lib_bool admitted;
    } inputs[] = {
        {LIB_TRUE,  0x1cu, 0x0fu, 1u, LIB_TRUE},
        {LIB_FALSE, 0x1cu, 0x0fu, 1u, LIB_FALSE},
        {LIB_TRUE,  0x0cu, 0x0fu, 1u, LIB_FALSE},
        {LIB_TRUE,  0x1du, 0x0fu, 1u, LIB_FALSE},
        {LIB_TRUE,  0x1cu, 0u,    1u, LIB_FALSE},
        {LIB_TRUE,  0x1cu, 0x0fu, 0u, LIB_FALSE}
    };
    const core_machine_fdc_config saved_config = fdc->connect.config;
    const core_machine_fdc_drive_bindings saved_drives = fdc->connect.drives;
    const lib_bool saved_present = media->present;
    const lib_size input_count = sizeof(inputs) / sizeof(inputs[0]);
    lib_i32 failed = 0;

    for (lib_size variant = 0u; variant < 2u * input_count; ++variant) {
        const lib_size input = variant % input_count;
        const lib_bool during_transfer = variant >= input_count;
        const lib_size initial = during_transfer ? 0u : input;
        for (lib_size command = 0u; command < sizeof(commands) / sizeof(commands[0]); ++command) {
            lib_u8 result[7];
            lib_bool mismatch;

            if (during_transfer && commands[command].phase == core_machine_fdc_PHASE_RESULT)
                continue;
            core_machine_fdc_reset(fdc);
            fdc->connect.config.ready_mask = inputs[initial].ready;
            fdc->connect.drives.installed_mask = inputs[initial].installed;
            media->present = inputs[initial].present;
            media->marks[0] = (commands[command].bytes[0] & 0x1fu) == 0x0cu ?
                CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA :
                CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
            core_machine_port_write(port, 0x03f2u, inputs[initial].dor);
            core_machine_fdc_command(fdc, port,
                (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
            core_machine_fdc_write_dma2(port, 0x0600u, 511u);
            core_machine_fdc_command(fdc, port, commands[command].bytes,
                commands[command].count);
            if (during_transfer) {
                failed |= fdc->data.phase != commands[command].phase;
                fdc->connect.config.ready_mask = inputs[input].ready;
                fdc->connect.drives.installed_mask = inputs[input].installed;
                media->present = inputs[input].present;
                core_machine_port_write(port, 0x03f2u, inputs[input].dor);
                core_machine_fdc_advance(fdc);
            }
            if (!inputs[input].admitted) {
                const lib_bool not_ready = inputs[input].ready == 0u;
                core_machine_fdc_advance(fdc);
                mismatch = !fdc->connect.irq_source.asserted ||
                    !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != (not_ready ? 0x48u : 0x40u) ||
                    result[1] != (not_ready ? 0u : 0x04u) ||
                    result[2] != 0u || fdc->connect.irq_source.asserted;
            } else {
                mismatch = fdc->data.phase != commands[command].phase ||
                    fdc->connect.irq_source.asserted !=
                        (commands[command].phase == core_machine_fdc_PHASE_RESULT);
            }
            mismatch |= core_machine_dma_has_pending_request(
                &machine->shared_dma_primary, &machine->shared_dma_secondary) !=
                (inputs[input].admitted &&
                    commands[command].phase != core_machine_fdc_PHASE_RESULT);
            if (mismatch) {
                fprintf(stderr, "FDC readiness variant=%zu command=%02x\n",
                    variant, commands[command].bytes[0]);
                failed = 1;
            }
        }
    }
    core_machine_fdc_reset(fdc);
    fdc->connect.config = saved_config;
    fdc->connect.drives = saved_drives;
    media->present = saved_present;
    return failed;
}

static lib_i32 core_machine_fdc_ready_edges(core_machine_fdc *fdc, t_port *port,
    core_machine_fdc_fixture_media *media)
{
    const lib_u8 saved_ready = fdc->connect.config.ready_mask;
    const lib_bool saved_present = media->present;
    lib_u8 result[7];
    lib_i32 failed = 0;

    fdc->connect.config.ready_mask = 0x0fu;
    core_machine_fdc_reset(fdc);
    core_machine_port_write(port, 0x03f2u, 0x1cu);
    core_machine_fdc_advance_at(fdc, fdc->data.reset_due_tick);
    for (lib_u8 drive = 0u; drive < 4u; ++drive) {
        core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
        failed |= !core_machine_fdc_read_result(fdc, port, result, 2u);
    }
    core_machine_fdc_command(fdc, port,
        (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
    media->present = LIB_FALSE;
    core_machine_fdc_refresh(fdc);
    failed |= fdc->connect.irq_source.asserted;
    media->present = LIB_TRUE;
    core_machine_fdc_refresh(fdc);
    failed |= fdc->connect.irq_source.asserted;

    /* Inject electrical transitions independently from media in this fixture.
     * Production's selected IBM/Compaq boards keep READY tied high. */
    fdc->connect.config.ready_mask = 0x0eu;
    core_machine_fdc_refresh(fdc);
    failed |= !fdc->connect.irq_source.asserted;
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0xc8u || fdc->connect.irq_source.asserted;
    fdc->connect.config.ready_mask = 0x0fu;
    core_machine_fdc_refresh(fdc);
    failed |= !fdc->connect.irq_source.asserted;
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0xc0u || fdc->connect.irq_source.asserted;
    /* All four causes survive unrelated ST3 reads and retain their own PCN. */
    fdc->connect.config.ready_mask = 0u;
    core_machine_fdc_refresh(fdc);
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x4au, 0u}, 2u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 7u) ||
        result[0] != 0x48u || !fdc->connect.irq_source.asserted;
    for (lib_u8 drive = 0u; drive < 4u; ++drive) {
        fdc->data.pcn[drive] = drive + 3u;
        core_machine_fdc_command(fdc, port, (const lib_u8[]){0x04u, drive}, 2u);
        failed |= !core_machine_fdc_read_result(fdc, port, result, 1u) ||
            !fdc->connect.irq_source.asserted;
        core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
        failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
            result[0] != (0xc8u | drive) || result[1] != drive + 3u ||
            !!fdc->connect.irq_source.asserted != (drive != 3u);
    }
    /* SEEK completion must not overwrite already observed READY changes. */
    fdc->connect.config.ready_mask = 0x0fu;
    core_machine_fdc_refresh(fdc);
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x0fu, 0u, 3u}, 3u);
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x20u || result[1] != 3u || !fdc->connect.irq_source.asserted;
    for (lib_u8 drive = 0u; drive < 4u; ++drive) {
        core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
        failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
            result[0] != (0xc0u | drive) || result[1] != drive + 3u;
    }
    failed |= fdc->connect.irq_source.asserted;
    fdc->connect.config.ready_mask = saved_ready;
    media->present = saved_present;
    core_machine_fdc_reset(fdc);
    if (failed) fprintf(stderr, "FDC READY edge/medium independence failed\n");
    return failed;
}

static lib_i32 core_machine_fdc_result_identity(core_machine_fdc *fdc,
    t_port *port, core_machine_fdc_fixture_media *media)
{
    const lib_bool saved_present = media->present;
    lib_i32 failed = 0;

    media->present = LIB_FALSE;
    for (lib_u8 identity = 0u; identity < 8u; ++identity) {
        lib_u8 result[7];
        const lib_u8 drive = identity & 3u;
        core_machine_fdc_reset(fdc);
        core_machine_port_write(port, 0x03f2u, 0x0cu | drive | (0x10u << drive));
        core_machine_fdc_command(fdc, port,
            (const lib_u8[]){0x4au, identity}, 2u);
        if (!core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
            result[0] != (0x40u | identity) || result[1] != 0x04u) {
            fprintf(stderr, "FDC ST0 identity=%u failed\n", identity);
            failed = 1;
        }
    }
    media->present = saved_present;
    core_machine_fdc_reset(fdc);
    return failed;
}

static lib_i32 core_machine_fdc_scan_status(core_machine_fdc *fdc, t_port *port,
    core_machine_fdc_fixture_media *fixture)
{
    /* Intel 8272A Table 10: rows are Equal, Low/Equal, High/Equal;
       columns are disk < host, disk == host, disk > host. These literal
       wire results deliberately do not reuse the implementation's masks. */
    static const lib_u8 commands[] = {0x51u, 0x59u, 0x5du};
    static const lib_u8 expected[][3] = {
        {0x04u, 0x08u, 0x04u},
        {0x00u, 0x08u, 0x04u},
        {0x04u, 0x08u, 0x00u}
    };
    const lib_u32 saved_rate = fdc->connect.config.clock_ticks_per_second;
    lib_i32 failed = 0;

    fdc->connect.config.clock_ticks_per_second = 0u;
    fixture->present = LIB_TRUE;
    fixture->marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
    fixture->forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK;
    for (lib_size mode = 0u; mode < 3u; ++mode) {
        for (lib_size relation = 0u; relation < 3u; ++relation) {
            lib_u8 result[7];
            lib_u8 command[] = {commands[mode], 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 1u};
            core_machine_fdc_reset(fdc);
            core_machine_port_write(port, 0x03f2u, 0x1cu);
            core_machine_fdc_command(fdc, port,
                (const lib_u8[]){0x03u, 0xdfu, 0x03u}, 3u);
            lib_memory_set(fixture->bytes, 0x40u + relation * 0x10u,
                sizeof(fixture->bytes));
            core_machine_fdc_command(fdc, port, command, sizeof(command));
            for (lib_size byte = 0u; byte < 512u; ++byte)
                core_machine_port_write(port, 0x03f5u, 0x50u);
            if (!core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                result[0] != 0x00u || result[2] != expected[mode][relation]) {
                fprintf(stderr, "FDC scan wire result mode=%u relation=%u\n",
                    (unsigned)mode, (unsigned)relation);
                failed = 1;
            }
        }
    }
    fdc->connect.config.clock_ticks_per_second = saved_rate;
    return failed;
}

static lib_i32 core_machine_fdc_scan_sequence(core_machine_fdc *fdc,
    t_port *port, core_machine_fdc_fixture_media *media)
{
    static const struct {
        lib_u8 step, eot, deleted, skip;
        lib_u8 bytes[3];
        lib_u16 compared;
        lib_u8 st1, st2, next_sector;
    } cases[] = {
        {1u, 3u, 0u, 0u, {0u, 1u, 1u}, 1024u, 0u, 0x08u, 3u},
        {2u, 3u, 0u, 0u, {0u, 1u, 1u}, 1024u, 0u, 0x08u, 5u},
        {2u, 2u, 0u, 0u, {0u, 1u, 1u}, 512u, 0x80u, 0x04u, 3u},
        {2u, 3u, 0u, 0u, {0u, 0u, 0u}, 1024u, 0u, 0x04u, 5u},
        {1u, 3u, 1u, 0u, {0u, 1u, 1u}, 512u, 0u, 0x44u, 2u},
        {1u, 3u, 1u, 1u, {0u, 1u, 0u}, 512u, 0u, 0x48u, 3u},
        {2u, 3u, 5u, 1u, {0u, 1u, 0u}, 0u, 0u, 0x44u, 5u},
        {2u, 2u, 1u, 1u, {0u, 1u, 0u}, 0u, 0x80u, 0x44u, 3u},
        {1u, 3u, 2u, 0u, {0u, 0u, 1u}, 1024u, 0u, 0x44u, 3u}
    };
    const core_machine_fdc_config saved_config = fdc->connect.config;
    lib_i32 failed = 0;

    fdc->connect.config.clock_ticks_per_second = 0u;
    fdc->connect.config.ready_mask = 0x0fu;
    fdc->drive_cylinder[0] = 0u;
    media->sector_count = 3u;
    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        lib_u8 result[7];
        lib_u16 compared = 0u;
        const lib_u32 reads = media->read_count;
        for (lib_u8 sector = 0u; sector < 3u; ++sector) {
            lib_memory_set(media->bytes + sector * 512u, cases[index].bytes[sector], 512u);
            media->marks[sector] = (cases[index].deleted & (1u << sector)) != 0u ?
                CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA : CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
        }
        core_machine_fdc_reset(fdc);
        core_machine_port_write(port, 0x03f2u, 0x1cu);
        core_machine_fdc_command(fdc, port, (const lib_u8[]){0x03u, 0xdfu, 3u}, 3u);
        core_machine_fdc_command(fdc, port, (const lib_u8[]){
            0x51u | (cases[index].skip ? 0x20u : 0u), 0u, 0u, 0u, 1u, 2u,
            cases[index].eot, 0x1bu, cases[index].step}, 9u);
        while (fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_SCAN && compared < 1536u) {
            core_machine_port_write(port, 0x03f5u, 1u);
            ++compared;
        }
        if (!core_machine_fdc_read_result(fdc, port, result, 7u) ||
            compared != cases[index].compared || media->read_count - reads != compared ||
            result[0] != (cases[index].st1 ? 0x40u : 0u) ||
            result[1] != cases[index].st1 || result[2] != cases[index].st2 ||
            result[5] != cases[index].next_sector) {
            fprintf(stderr, "FDC SCAN sequence case=%zu bytes=%u status=%02x/%02x/%02x R=%u\n",
                index, compared, result[0], result[1], result[2], result[5]);
            failed = 1;
        }
    }
    media->sector_count = 1u;
    media->marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
    fdc->connect.config = saved_config;
    core_machine_fdc_reset(fdc);
    return failed;
}

static lib_i32 core_machine_fdc_read_sequence(core_machine_fdc *fdc,
    t_port *port, core_machine_fdc_fixture_media *media)
{
    static const struct {
        lib_u8 mismatched, skip, delivered, st1, st2;
    } cases[] = {
        {0u, 0u, 7u, 0u, 0u},
        {1u, 0u, 1u, 0u, 0x40u},
        {2u, 0u, 3u, 0u, 0x40u},
        {1u, 1u, 6u, 0u, 0u},
        {2u, 1u, 5u, 0u, 0u},
        {7u, 1u, 0u, 0x80u, 0u}
    };
    const core_machine_fdc_config saved_config = fdc->connect.config;
    lib_i32 failed = 0;

    fdc->connect.config.clock_ticks_per_second = 0u;
    fdc->connect.config.ready_mask = 0x0fu;
    fdc->drive_cylinder[0] = 0u;
    media->sector_count = 3u;
    for (lib_u8 deleted = 0u; deleted < 2u; ++deleted) {
        for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
            lib_u8 result[7];
            const lib_u32 reads = media->read_count;
            lib_u32 expected = 0u;
            for (lib_u8 sector = 0u; sector < 3u; ++sector) {
                lib_memory_set(media->bytes + sector * 512u, sector + 1u, 512u);
                media->marks[sector] = (deleted != 0u) !=
                    ((cases[index].mismatched & (1u << sector)) != 0u) ?
                    CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA : CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
            }
            core_machine_fdc_reset(fdc);
            core_machine_port_write(port, 0x03f2u, 0x1cu);
            core_machine_fdc_command(fdc, port, (const lib_u8[]){0x03u, 0xdfu, 3u}, 3u);
            core_machine_fdc_command(fdc, port, (const lib_u8[]){
                (deleted ? 0x4cu : 0x46u) | (cases[index].skip ? 0x20u : 0u),
                0u, 0u, 0u, 1u, 2u, 3u, 0x1bu, 0xffu}, 9u);
            for (lib_u8 sector = 0u; sector < 3u; ++sector) {
                if ((cases[index].delivered & (1u << sector)) == 0u) continue;
                for (lib_u16 byte = 0u; byte < 512u; ++byte) {
                    failed |= fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_READ ||
                        core_machine_port_read(port, 0x03f5u) != sector + 1u;
                    ++expected;
                }
            }
            if (!core_machine_fdc_read_result(fdc, port, result, 7u) ||
                media->read_count - reads != expected ||
                result[0] != (cases[index].st1 ? 0x40u : 0u) ||
                result[1] != cases[index].st1 || result[2] != cases[index].st2) {
                fprintf(stderr, "FDC READ sequence deleted=%u case=%zu\n", deleted, index);
                failed = 1;
            }
        }
    }
    media->sector_count = 1u;
    media->marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
    fdc->connect.config = saved_config;
    core_machine_fdc_reset(fdc);
    return failed;
}

static lib_i32 core_machine_fdc_write_terminal(core_machine *machine,
    core_machine_fdc_fixture_media *media)
{
    core_machine_fdc *fdc = &machine->fdc;
    t_port *port = &machine->executor_port;
    const core_machine_fdc_config saved_config = fdc->connect.config;
    enum { FINISH, RESET, MOTOR_OFF, WRITE_FAILURE };
    static const struct { lib_u16 length; lib_u8 action; } cases[] = {
        {1u, FINISH}, {511u, FINISH}, {512u, FINISH},
        {1u, RESET}, {1u, MOTOR_OFF}, {1u, WRITE_FAILURE}
    };
    lib_u8 source[512];
    lib_i32 failed = 0;

    lib_memory_set(source, 0xa5u, sizeof(source));
    failed |= core_machine_memory_write_physical(&machine->executor_memory,
        0x0600u, (lib_uptr)source, sizeof(source)) != LIB_STATUS_OK;
    fdc->connect.config.ready_mask = 0x0fu;
    fdc->drive_cylinder[0] = 0u;
    media->sector_count = 3u;
    for (lib_u8 deleted = 0u; deleted < 2u; ++deleted) {
        for (lib_u8 timed = 0u; timed < 2u; ++timed) {
            fdc->connect.config.clock_ticks_per_second = timed ? 8000000u : 0u;
            for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
                const lib_u16 length = cases[index].length;
                lib_u8 result[7];
                const lib_u32 writes = media->write_count;
                lib_memory_set(media->bytes, 0x5au, sizeof(media->bytes));
                core_machine_fdc_reset(fdc);
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                core_machine_fdc_command(fdc, port, (const lib_u8[]){0x03u, 0xdfu, 2u}, 3u);
                core_machine_port_write(port, 0x03f7u, VFDC_CCR_RATE_500);
                core_machine_fdc_write_dma2(port, 0x0600u, length - 1u);
                core_machine_port_write(port, 0x000bu, 0x4au);
                core_machine_fdc_command(fdc, port, (const lib_u8[]){
                    deleted ? 0x49u : 0x45u, 0u, 0u, 0u, 1u, 2u, 3u, 0x1bu, 0xffu}, 9u);
                for (lib_u16 byte = 0u; byte < length; ++byte) {
                    test_dma_transfers(&machine->shared_dma_latch,
                        &machine->shared_dma_primary, &machine->shared_dma_secondary,
                        &machine->executor_memory, port, 1u);
                    if (byte + 1u < length)
                        core_machine_fdc_advance_at(fdc, fdc->data.next_dma_byte_tick);
                }
                failed |= media->write_count - writes != length;
                if (cases[index].action != FINISH) {
                    const lib_u64 due = fdc->data.next_dma_byte_tick;
                    if (cases[index].action == RESET)
                        core_machine_port_write(port, 0x03f2u, 0u);
                    else if (cases[index].action == MOTOR_OFF)
                        core_machine_port_write(port, 0x03f2u, 0x0cu);
                    else media->forced_write_result = CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
                    core_machine_fdc_advance_at(fdc, due);
                    if (cases[index].action == RESET)
                        failed |= fdc->data.phase != core_machine_fdc_PHASE_COMMAND ||
                            fdc->connect.irq_source.asserted;
                    else failed |= !core_machine_fdc_read_result(fdc, port, result, 7u) ||
                        result[0] != 0x40u || result[1] !=
                            (cases[index].action == MOTOR_OFF ? 0x04u : 0x02u);
                    failed |= media->write_count - writes != length ||
                        fdc->data.dma_byte_gate_pending ||
                        core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                            &machine->shared_dma_secondary);
                    media->forced_write_result = CORE_MACHINE_MEDIA_RESULT_OK;
                    for (lib_u16 byte = 0u; byte < sizeof(media->bytes); ++byte)
                        failed |= media->bytes[byte] != (byte < length ? 0xa5u : 0x5au);
                    if (failed) fprintf(stderr, "FDC WRITE TC interruption=%u\n", cases[index].action);
                    continue;
                }
                for (lib_u16 byte = length; byte < 512u; ++byte) {
                    const lib_u64 due = fdc->data.next_dma_byte_tick;
                    const lib_u32 before = media->write_count;
                    failed |= fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_WRITE_TAIL ||
                        core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                            &machine->shared_dma_secondary) ||
                        (core_machine_port_read(port, 0x03f4u) & VFDC_MSR_RQM) != 0u;
                    core_machine_fdc_advance_at(fdc, due - 1u);
                    failed |= media->write_count != before;
                    core_machine_fdc_advance_at(fdc, due);
                    failed |= media->write_count != before + 1u;
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, 7u) ||
                    result[0] != 0u || result[1] != 0u || result[2] != 0u ||
                    media->write_count - writes != 512u || fdc->data.dma_byte_gate_pending ||
                    media->marks[0] != (deleted ? CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA :
                        CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA);
                for (lib_u16 byte = 0u; byte < sizeof(media->bytes); ++byte)
                    failed |= media->bytes[byte] != (byte < length ? 0xa5u :
                        (byte < 512u ? 0u : 0x5au));
                if (failed) fprintf(stderr, "FDC WRITE TC deleted=%u timed=%u length=%u\n",
                    deleted, timed, length);
            }
        }
    }
    media->sector_count = 1u;
    media->marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
    fdc->connect.config = saved_config;
    core_machine_fdc_reset(fdc);
    return failed;
}

static lib_i32 core_machine_fdc_terminal_id(core_machine *machine,
    core_machine_fdc_fixture_media *media)
{
    static const lib_u8 commands[] = {0x46u, 0x4cu, 0x45u, 0x49u};
    core_machine_fdc *fdc = &machine->fdc;
    t_port *port = &machine->executor_port;
    const core_machine_fdc_config saved_config = fdc->connect.config;
    lib_i32 failed = 0;

    fdc->connect.config.clock_ticks_per_second = 0u;
    fdc->connect.config.ready_mask = 0x0fu;
    fdc->drive_cylinder[0] = 0u;
    media->head_count = 2u;
    for (lib_size command = 0u; command < sizeof(commands); ++command) {
        for (lib_u8 variant = 0u; variant < 16u; ++variant) {
            const lib_u16 length = (variant & 1u) != 0u ? 512u : 1u;
            const lib_bool multi_track = (variant & 2u) != 0u;
            const lib_u8 head = (variant >> 2u) & 1u;
            const lib_bool at_eot = (variant & 8u) != 0u;
            lib_u8 result[7];

            core_machine_fdc_reset(fdc);
            media->marks[head] = commands[command] == 0x4cu ?
                CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA : CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
            core_machine_port_write(port, 0x03f2u, 0x1cu);
            core_machine_fdc_command(fdc, port, (const lib_u8[]){0x03u, 0xdfu, 2u}, 3u);
            core_machine_fdc_write_dma2(port, 0x0600u, length - 1u);
            if (command >= 2u) core_machine_port_write(port, 0x000bu, 0x4au);
            core_machine_fdc_command(fdc, port, (const lib_u8[]){
                commands[command] | (multi_track ? 0x80u : 0u), head << 2u,
                0u, head, 1u, 2u, at_eot ? 1u : 2u, 0x1bu, 0xffu}, 9u);
            for (lib_u16 byte = 0u; byte < length; ++byte) {
                test_dma_transfers(&machine->shared_dma_latch,
                    &machine->shared_dma_primary, &machine->shared_dma_secondary,
                    &machine->executor_memory, port, 1u);
                if (byte + 1u < length)
                    core_machine_fdc_advance_at(fdc, fdc->data.next_dma_byte_tick);
            }
            for (lib_u16 byte = length; byte < 512u &&
                fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_WRITE_TAIL; ++byte)
                core_machine_fdc_advance_at(fdc, fdc->data.next_dma_byte_tick);
            if (!core_machine_fdc_read_result(fdc, port, result, 7u) ||
                (result[0] & 0xc0u) != 0u || result[1] != 0u || result[2] != 0u ||
                result[3] != (at_eot && (!multi_track || head != 0u) ? 1u : 0u) ||
                result[4] != (at_eot && multi_track ? head ^ 1u : head) ||
                result[5] != (at_eot ? 1u : 2u) || result[6] != 2u ||
                fdc->drive_cylinder[0] != 0u || fdc->data.pcn[0] != 0u) {
                fprintf(stderr, "FDC TC ID command=%02x variant=%u CHRN=%u/%u/%u/%u\n",
                    commands[command], variant, result[3], result[4], result[5], result[6]);
                failed = 1;
            }
        }
    }
    media->head_count = 1u;
    media->marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
    fdc->connect.config = saved_config;
    core_machine_fdc_reset(fdc);
    return failed;
}

static lib_i32 core_machine_fdc_no_implied_seek(core_machine_fdc *fdc,
    t_port *port, core_machine_fdc_fixture_media *media)
{
    const lib_u8 commands[] = {0x46u, 0x45u, 0x4cu, 0x49u, 0x51u, 0x59u, 0x5du, 0x42u};
    const lib_u32 reads = media->read_count;
    const lib_u32 writes = media->write_count;
    lib_u8 result[7];
    lib_i32 failed = 0;

    fdc->drive_cylinder[0] = 0u;
    media->cylinder_count = 2u;
    for (lib_size index = 0u; index < sizeof(commands); ++index) {
        core_machine_fdc_reset(fdc);
        core_machine_port_write(port, 0x03f2u, 0x1cu);
        core_machine_fdc_command(fdc, port,
            (const lib_u8[]){commands[index], 0u, 1u, 0u, 1u, 2u, 1u, 0x1bu, 1u}, 9u);
        failed |= !core_machine_fdc_read_result(fdc, port, result, 7u) ||
            result[0] != 0x40u || result[1] != 0x04u ||
            result[2] != (commands[index] == 0x42u ? 0u : 0x10u) ||
            result[3] != (commands[index] == 0x42u ? 1u : 0u) ||
            fdc->drive_cylinder[0] != 0u || fdc->data.pcn[0] != 0u;
    }
    failed |= media->read_count != reads || media->write_count != writes;
    /* READ ID must ignore the previous command's requested cylinder. */
    fdc->data.cylinder = 1u;
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x4au, 0u}, 2u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 7u) ||
        result[0] != 0u || result[3] != 0u;
    media->cylinder_count = 1u;
    core_machine_fdc_reset(fdc);
    if (failed) fputs("FDC data command implied a seek or READ ID used stale C\n", stderr);
    return failed;
}

static lib_i32 core_machine_fdc_seek_pins(core_machine_fdc *fdc, t_port *port)
{
    const core_machine_fdc_drive_bindings saved_drives = fdc->connect.drives;
    const core_machine_fdc_config saved_config = fdc->connect.config;
    lib_u8 result[2];
    lib_i32 failed = 0;

    fdc->connect.drives.installed_mask = 1u;
    fdc->connect.drives.cylinder_count[0] = 40u;
    fdc->connect.drives.track_zero_active_low_mask = 0u;
    fdc->connect.config.ready_mask = 0x0fu;
    fdc->connect.config.clock_ticks_per_second = 8000000u;
    fdc->drive_cylinder[0] = 0u;
    core_machine_fdc_reset(fdc);
    core_machine_port_write(port, 0x03f2u, 0x1cu);
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x0fu, 0u, 100u}, 3u);
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x20u || result[1] != 100u || fdc->drive_cylinder[0] != 39u;

    /* Reset changes PCN, not the drive's physical position or Track0. */
    core_machine_fdc_reset(fdc);
    failed |= fdc->data.pcn[0] != 0u || fdc->drive_cylinder[0] != 39u;
    core_machine_port_write(port, 0x03f2u, 0x1cu);
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x04u, 0u}, 2u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 1u) || (result[0] & 0x10u);
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x07u, 0u}, 2u);
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x20u || result[1] != 0u || fdc->data.seek_steps[0] != 39u ||
        fdc->drive_cylinder[0] != 0u;

    /* No Track0 within 77 pulses is EC; a second command can finish. */
    fdc->connect.drives.cylinder_count[0] = 80u;
    fdc->drive_cylinder[0] = 79u;
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x07u, 0u}, 2u);
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x70u || result[1] != 0u || fdc->data.seek_steps[0] != 77u ||
        fdc->drive_cylinder[0] != 2u;
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x07u, 0u}, 2u);
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x20u || fdc->data.seek_steps[0] != 2u || fdc->drive_cylinder[0] != 0u;

    /* READY loss aborts at the current PCN, not at the requested target. */
    core_machine_fdc_submit(fdc, port, (const lib_u8[]){0x0fu, 0u, 5u}, 3u);
    core_machine_fdc_advance_at(fdc, fdc->data.seek_due_tick[0]);
    core_machine_fdc_advance_at(fdc, fdc->data.seek_due_tick[0]);
    fdc->connect.config.ready_mask = 0x0eu;
    core_machine_fdc_advance(fdc);
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x68u || result[1] != 2u || fdc->drive_cylinder[0] != 2u;
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x0fu, 4u, 9u}, 3u);
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x6cu || result[1] != 2u || fdc->data.seek_steps[0] != 0u;

    /* DOR routes STEP to mechanism zero even when the chip counts unit one. */
    fdc->connect.config.ready_mask = 0x0fu;
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x0fu, 5u, 1u}, 3u);
    core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x25u || result[1] != 1u || fdc->drive_cylinder[0] != 3u;
    fdc->connect.drives = saved_drives;
    fdc->connect.config = saved_config;
    core_machine_fdc_reset(fdc);
    if (failed) fputs("FDC PCN/Track0/READY qualification failed\n", stderr);
    return failed;
}

static lib_i32 core_machine_fdc_seek_ownership(core_machine_fdc *fdc, t_port *port)
{
    const core_machine_fdc_drive_bindings saved_drives = fdc->connect.drives;
    const lib_u32 saved_rate = fdc->connect.config.clock_ticks_per_second;
    lib_u8 result[2];
    lib_i32 failed = 0;

    /* Use absent mechanics to distinguish SEEK from RECALIBRATE under the
       existing model. READY/Track0 qualification is a separate cutover gate. */
    fdc->connect.drives.installed_mask = 0u;
    fdc->connect.config.clock_ticks_per_second = 8000000u;
    for (lib_u8 drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        for (lib_u8 recalibrate = 0u; recalibrate < 2u; ++recalibrate) {
            core_machine_fdc_reset(fdc);
            core_machine_port_write(port, 0x03f2u, 0xfcu | drive);
            core_machine_fdc_command(fdc, port,
                (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
            core_machine_fdc_command(fdc, port,
                (const lib_u8[]){0x0fu, drive, 4u}, 3u);
            core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
            failed |= !core_machine_fdc_read_result(fdc, port, result, 2u);
            if (recalibrate) {
                core_machine_fdc_submit(fdc, port,
                    (const lib_u8[]){0x07u, drive}, 2u);
            } else {
                core_machine_fdc_submit(fdc, port,
                    (const lib_u8[]){0x0fu, drive, 6u}, 3u);
            }
            core_machine_fdc_submit(fdc, port,
                (const lib_u8[]){0x04u, drive}, 2u);
            failed |= !core_machine_fdc_read_result(fdc, port, result, 1u);
            core_machine_fdc_finish_seeks(fdc);
            core_machine_fdc_submit(fdc, port, (const lib_u8[]){0x08u}, 1u);
            failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                result[0] != ((recalibrate ? 0x70u : 0x20u) | drive) ||
                result[1] != (recalibrate ? 0u : 6u);
        }
    }
    if (failed) fputs("FDC seek identity changed by intervening command\n", stderr);

    /* Mixed commands finish at their own deadlines: absent-mechanism
       recalibration requires 77 pulses, not SEEK's four PCN steps. */
    core_machine_fdc_reset(fdc);
    core_machine_port_write(port, 0x03f2u, 0xfcu);
    core_machine_fdc_command(fdc, port,
        (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
    for (lib_u8 drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        core_machine_fdc_command(fdc, port,
            (const lib_u8[]){0x0fu, drive, 4u}, 3u);
        core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
        failed |= !core_machine_fdc_read_result(fdc, port, result, 2u);
    }
    for (lib_u8 drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        const lib_u8 command[] = {(drive & 1u) ? 0x07u : 0x0fu, drive, 8u};
        core_machine_fdc_submit(fdc, port, command, (drive & 1u) ? 2u : 3u);
    }
    core_machine_fdc_finish_seeks(fdc);
    for (lib_u8 index = 0u; index < CORE_MACHINE_FDC_DRIVE_COUNT; ++index) {
        const lib_u8 drive = (const lib_u8[]){0u, 2u, 1u, 3u}[index];
        core_machine_fdc_submit(fdc, port, (const lib_u8[]){0x08u}, 1u);
        failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
            result[0] != (((drive & 1u) ? 0x70u : 0x20u) | drive) ||
            result[1] != ((drive & 1u) ? 0u : 8u);
    }

    /* A second command cannot replace the same unit's active operation. */
    for (lib_u8 drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        lib_u64 due;
        core_machine_fdc_reset(fdc);
        core_machine_port_write(port, 0x03f2u, 0xfcu | drive);
        core_machine_fdc_command(fdc, port,
            (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
        core_machine_fdc_submit(fdc, port,
            (const lib_u8[]){0x0fu, drive, 3u}, 3u);
        due = fdc->data.seek_due_tick[drive];
        core_machine_fdc_submit(fdc, port,
            (const lib_u8[]){0x0fu, drive, 7u}, 3u);
        if (fdc->data.phase != core_machine_fdc_PHASE_RESULT ||
            fdc->data.seek_due_tick[drive] != due ||
            fdc->data.seek_target[drive] != 3u ||
            !core_machine_fdc_read_result(fdc, port, result, 1u) ||
            result[0] != 0x80u || fdc->connect.irq_source.asserted) {
            fputs("FDC duplicate unit seek replaced active request\n", stderr);
            failed = 1;
        }
    }

    /* Fill the four legal outstanding slots. On the old implementation the
       fifth request is inspected before its deadline, never executing the
       out-of-bounds append as part of the negative control. */
    core_machine_fdc_reset(fdc);
    core_machine_port_write(port, 0x03f2u, 0xfcu);
    core_machine_fdc_command(fdc, port,
        (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
    for (lib_u8 drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        core_machine_fdc_submit(fdc, port,
            (const lib_u8[]){0x0fu, drive, 1u}, 3u);
    }
    core_machine_fdc_advance_at(fdc, fdc->data.seek_due_tick[3u]);
    failed |= fdc->data.seek_result_count != CORE_MACHINE_FDC_DRIVE_COUNT;
    core_machine_fdc_submit(fdc, port, (const lib_u8[]){0x0fu, 0u, 2u}, 3u);
    if (fdc->data.phase != core_machine_fdc_PHASE_RESULT ||
        fdc->data.seek_pending[0u] ||
        !core_machine_fdc_read_result(fdc, port, result, 1u) || result[0] != 0x80u) {
        fputs("FDC accepted fifth undrained seek completion\n", stderr);
        failed = 1;
    } else {
        static const lib_u8 blocked[][2] = {
            {0x03u, 3u}, {0x04u, 2u}, {0x07u, 2u}, {0x0fu, 3u},
            {0x0au, 2u}, {0x05u, 9u}, {0x06u, 9u}, {0x09u, 9u},
            {0x0cu, 9u}, {0x11u, 9u}, {0x19u, 9u}, {0x1du, 9u},
            {0x0du, 6u}, {0x02u, 9u}, {0x00u, 1u}
        };
        for (lib_size index = 0u; index < sizeof(blocked) / sizeof(blocked[0]); ++index) {
            lib_u8 command[9] = {0};
            command[0] = blocked[index][0];
            core_machine_fdc_submit(fdc, port, command, blocked[index][1]);
            failed |= !core_machine_fdc_read_result(fdc, port, result, 1u) ||
                result[0] != 0x80u ||
                fdc->data.seek_result_count != CORE_MACHINE_FDC_DRIVE_COUNT;
        }
        for (lib_u8 drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
            core_machine_fdc_submit(fdc, port, (const lib_u8[]){0x08u}, 1u);
            failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                result[0] != (0x20u | drive) || result[1] != 1u;
        }
        failed |= fdc->data.seek_result_count != 0u;
    }
    core_machine_fdc_reset(fdc);
    failed |= fdc->data.seek_result_count != 0u || fdc->connect.irq_source.asserted;
    for (lib_u8 drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive)
        failed |= fdc->data.seek_pending[drive];
    fdc->connect.drives = saved_drives;
    fdc->connect.config.clock_ticks_per_second = saved_rate;
    return failed;
}

lib_i32 main(void)
{
    static const lib_u8 specify_non_dma[] = {0x03u, 0xdfu, 0x03u};
    static const lib_u8 read_sector[] = {
        0xc6u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const lib_u8 read_sector_dma_terminal[] = {
        0xe6u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x0fu, 0x2au, 0xffu
    };
    static const lib_u8 write_sector[] = {
        0xc5u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const lib_u8 read_deleted_sector[] = {
        0xccu, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const lib_u8 write_deleted_sector[] = {
        0xc9u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const lib_u8 scan_equal[] = {
        0x11u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const lib_u8 scan_low_or_equal[] = {
        0x19u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const lib_u8 scan_high_or_equal[] = {
        0x1du, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const lib_u8 scan_equal_skip[] = {
        0x31u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const lib_u8 format_track[] = {
        0x4du, 0x00u, 0x02u, 0x01u, 0x1bu, 0xa5u
    };
    static const lib_u8 format_id[] = {0x00u, 0x00u, 0x01u, 0x02u};
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_fdc_config fdc_config = {
        .dor_port = 0x03f2u, .status_port = 0x03f4u, .data_port = 0x03f5u,
        .direction_port = 0x03f7u, .control_port = 0x03f7u,
        .irq = 6u, .dma_channel = 2u, .ready_mask = 0x0fu,
        .clock_ticks_per_second = 8000000u,
        .diagnostic_port = 0x03f1u, .diagnostic_read_value = 0x50u
    };
    const core_machine_fdc_drive_bindings drives = {
        {1u, CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID}, 0x01u, 0x01u, {0u, 0u, 0u, 0u}, 0u, {0}
    };
    const core_machine_dma_wiring dma_wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_fdc_fixture_media fixture = {
        .generation = 1u, .present = LIB_TRUE, .sector_count = 1u, .cylinder_count = 1u, .head_count = 1u,
        .forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK,
        .forced_write_result = CORE_MACHINE_MEDIA_RESULT_OK,
        .forced_format_result = CORE_MACHINE_MEDIA_RESULT_OK
    };
    core_machine_media_registry *media = LIB_NULL;
    core_machine_dma_request_binding dma_request = {0};
    core_machine_fdc_topology topology = {0};
    core_machine *machine = LIB_NULL;
    core_machine_fdc *fdc;
    t_port *port;
    lib_u8 result[7];
    lib_u8 scan_dma[512];
    lib_u64 ndma_gate_tick;
    lib_u32 fallback_read_count;
    lib_i32 failed = 0;

    fixture.bytes[0] = 0x4au;
    if (core_machine_media_registry_create(&media) != LIB_STATUS_OK ||
        core_machine_create(&config, &machine) != LIB_STATUS_OK) failed |= 0x01;
    if (!failed) {
        fdc = &machine->fdc;
        port = &machine->executor_port;
        if (fdc == LIB_NULL || port == LIB_NULL ||
            core_machine_media_registry_bind(media, 1u, &fixture,
                &core_machine_fdc_fixture_provider) != LIB_STATUS_OK ||
            core_machine_media_registry_freeze(media) != LIB_STATUS_OK ||
            core_machine_media_registry_bind(media, 2u, &fixture,
                &core_machine_fdc_fixture_provider) != LIB_STATUS_INVALID_STATE ||
            core_machine_configure_dma(machine, &dma_wiring, &dma_request) !=
                LIB_STATUS_OK) {
            failed |= 0x02;
        } else {
            topology.media_registry = media;
            topology.drives = drives;
            topology.dma_request = dma_request;
            topology.config = fdc_config;
            if (core_machine_configure_fdc(machine, &topology) != LIB_STATUS_OK ||
                core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                core_machine_reset(machine) != LIB_STATUS_OK) {
                failed |= 0x04;
            } else {
                failed |= core_machine_port_read(port, fdc_config.diagnostic_port) != 0x50u;
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                failed |= fdc->connect.irq_source.asserted ||
                    !fdc->data.reset_pending || fdc->data.reset_due_tick != 8192u;
                core_machine_fdc_advance_at(fdc, 8191u);
                failed |= fdc->connect.irq_source.asserted;
                core_machine_fdc_advance_at(fdc, 8192u);
                failed |= !fdc->connect.irq_source.asserted;
                for (lib_u8 reset_drive = 0u;
                    reset_drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++reset_drive) {
                    core_machine_fdc_command(fdc, port,
                        (const lib_u8[]){0x08u}, 1u);
                    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                        result[0] != (core_machine_fdc_ST0_READY_CHANGE | reset_drive) ||
                        result[1] != 0u || fdc->connect.irq_source.asserted;
                }
                core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
                failed |= !core_machine_fdc_read_result(fdc, port, result, 1u) ||
                    result[0] != 0x80u || fdc->connect.irq_source.asserted;
                /* A new Recalibrate supersedes an undrained reset notice;
                   Sense Interrupt must report the completed operation. */
                core_machine_port_write(port, fdc_config.dor_port, 0u);
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_advance_at(fdc, fdc->data.reset_due_tick);
                core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x07u, 0u}, 2u);
                core_machine_fdc_advance(fdc);
                core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x08u}, 1u);
                failed |= fdc->data.reset_sense_mask != 0u ||
                    !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                    result[0] != VFDC_ST0_SEEK_END || result[1] != 0u;
                core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));
                core_machine_port_write(port, fdc_config.control_port, VFDC_CCR_RATE_250);

                core_machine_fdc_command(fdc, port, (const lib_u8[]){0x10u}, 1u);
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
                for (lib_u8 reset_drive = 0u;
                    reset_drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++reset_drive) {
                    core_machine_fdc_command(fdc, port,
                        (const lib_u8[]){0x08u}, 1u);
                    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                        result[0] != (core_machine_fdc_ST0_READY_CHANGE | reset_drive) ||
                        result[1] != 0u || fdc->connect.irq_source.asserted;
                }
                core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));

                /* PCN advances once per step; IRQ remains absent until the
                   final source-labelled 3-ms-per-track deadline. */
                core_machine_port_write(port, fdc_config.data_port, 0x0fu);
                core_machine_port_write(port, fdc_config.data_port, 0x00u);
                core_machine_port_write(port, fdc_config.data_port, 0x03u);
                core_machine_fdc_advance_at(fdc, 100u);
                failed |= fdc->data.seek_pending[0u] == LIB_FALSE ||
                    fdc->data.pcn[0u] != 0u || fdc->connect.irq_source.asserted ||
                    fdc->data.seek_due_tick[0u] != 24100u;
                core_machine_fdc_advance_at(fdc, 72099u);
                failed |= fdc->data.seek_pending[0u] == LIB_FALSE ||
                    fdc->connect.irq_source.asserted || fdc->data.pcn[0u] != 2u;
                core_machine_fdc_advance_at(fdc, 72100u);
                failed |= fdc->data.pcn[0u] != 3u || !fdc->connect.irq_source.asserted;
                core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
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

                /* An installed empty drive can seek: media availability
                   controls sector transfer, not the mechanical completion. */
                fdc->connect.drives.installed_mask |= 0x02u;
                core_machine_port_write(port, fdc_config.dor_port, 0x2du);
                core_machine_fdc_command(fdc, port, (const lib_u8[]){0x07u, 1u}, 2u);
                core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
                failed |= !core_machine_fdc_read_result(fdc, port, result, 2u);
                core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x0fu, 0x01u, 0x01u}, 3u);
                core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
                if (!core_machine_fdc_read_result(fdc, port, result, 2u) ||
                    result[0] != (core_machine_fdc_ST0_NORMAL |
                    VFDC_ST0_SEEK_END | 1u) || result[1] != 1u) failed |= 0x100;
                /* ST3 observes the selected drive's Track-0 input, not the
                   controller's most recently completed seek. */
                fdc->drive_cylinder[0u] = 0u;
                fdc->data.cylinder = 0u;
                core_machine_port_write(port, fdc_config.dor_port, 0x2du);
                core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x04u, 0x01u}, 2u);
                if (!core_machine_fdc_read_result(fdc, port, result, 1u) ||
                    result[0] != 0x21u) failed |= 0x200;
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);

                for (lib_u32 index = 0u; index < sizeof(read_sector); ++index) {
                    core_machine_port_write(port, fdc_config.data_port, read_sector[index]);
                }
                failed |= fdc->data.phase != core_machine_fdc_PHASE_PENDING_COMMAND ||
                    core_machine_port_read(port, fdc_config.status_port) != VFDC_MSR_CB ||
                    fdc->connect.irq_source.asserted;
                core_machine_fdc_advance(fdc);
                failed |= fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_READ;
                failed |= core_machine_port_read(port, fdc_config.data_port) != 0x4au;
                for (lib_u32 index = 1u; index < 512u; ++index) {
                    (void)core_machine_port_read(port, fdc_config.data_port);
                }
                failed |= fdc->data.phase != core_machine_fdc_PHASE_PENDING_COMPLETE ||
                    core_machine_port_read(port, fdc_config.status_port) != VFDC_MSR_CB ||
                    fdc->connect.irq_source.asserted;
                core_machine_fdc_advance(fdc);
                failed |= fdc->data.phase != core_machine_fdc_PHASE_RESULT ||
                    !fdc->connect.irq_source.asserted ||
                    !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != core_machine_fdc_ST0_NORMAL ||
                    fdc->connect.irq_source.asserted;
                fixture.read_count = 0u;

                core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                failed |= core_machine_port_read(port, fdc_config.data_port) != 0x4au;
                for (lib_u32 index = 1u; index < 512u; ++index) {
                    (void)core_machine_port_read(port, fdc_config.data_port);
                }
                core_machine_fdc_advance(fdc);
                failed |= !fdc->connect.irq_source.asserted ||
                    !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != core_machine_fdc_ST0_NORMAL || fixture.read_count != 512u ||
                    fdc->connect.irq_source.asserted;

                fixture.marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA;
                core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    (void)core_machine_port_read(port, fdc_config.data_port);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & VFDC_ST2_CONTROL_MARK) == 0u;
                core_machine_fdc_command(fdc, port, read_deleted_sector,
                    sizeof(read_deleted_sector));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    (void)core_machine_port_read(port, fdc_config.data_port);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & VFDC_ST2_CONTROL_MARK) != 0u;

                fixture.marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
                core_machine_fdc_command(fdc, port, write_deleted_sector,
                    sizeof(write_deleted_sector));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x6bu : 0u);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != core_machine_fdc_ST0_NORMAL ||
                    fixture.marks[0] != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA ||
                    fixture.bytes[0] != 0x6bu;

                fixture.write_count = 0u;
                core_machine_fdc_command(fdc, port, write_sector, sizeof(write_sector));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x5au : 0u);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != core_machine_fdc_ST0_NORMAL || fixture.write_count != 512u ||
                    fixture.bytes[0] != 0x5au;

                /* Scan commands receive comparison bytes through the same
                   host-to-controller path as a write, but never mutate media.
                   FFh is the documented no-care compare byte. */
                fixture.marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
                core_machine_fdc_command(fdc, port, scan_equal, sizeof(scan_equal));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x5au : 0xffu);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (VFDC_ST2_SCAN_MATCH | VFDC_ST2_SCAN_MISMATCH)) !=
                        VFDC_ST2_SCAN_MATCH || fixture.bytes[0] != 0x5au;
                core_machine_fdc_command(fdc, port, scan_low_or_equal,
                    sizeof(scan_low_or_equal));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x60u : 0xffu);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (VFDC_ST2_SCAN_MATCH | VFDC_ST2_SCAN_MISMATCH)) !=
                        0u;
                core_machine_fdc_command(fdc, port, scan_high_or_equal,
                    sizeof(scan_high_or_equal));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x50u : 0xffu);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (VFDC_ST2_SCAN_MATCH | VFDC_ST2_SCAN_MISMATCH)) !=
                        0u;
                core_machine_fdc_command(fdc, port, scan_equal, sizeof(scan_equal));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x50u : 0xffu);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (VFDC_ST2_SCAN_MATCH | VFDC_ST2_SCAN_MISMATCH)) !=
                        VFDC_ST2_SCAN_MISMATCH;
                fixture.marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA;
                core_machine_fdc_command(fdc, port, scan_equal, sizeof(scan_equal));
                for (lib_u32 index = 0u; index < 512u; ++index) {
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
                    VFDC_ST2_CONTROL_MARK)) != (VFDC_ST2_SCAN_MISMATCH | VFDC_ST2_CONTROL_MARK);
                fixture.marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;

                core_machine_fdc_command(fdc, port, format_track, sizeof(format_track));
                core_machine_fdc_command(fdc, port, format_id, sizeof(format_id));
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != core_machine_fdc_ST0_NORMAL || fixture.format_count != 1u ||
                    fixture.generation != 2u || fixture.bytes[511] != 0xa5u;

                core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x0fu, 0x00u, 0x00u}, 3u);
                core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
                failed |= !core_machine_fdc_read_result(fdc, port, result, 2u);
                core_machine_fdc_refresh(fdc);
                failed |= (core_machine_port_read(port, fdc_config.direction_port) & VFDC_DIR_DC) != 0u;
                ++fixture.generation;
                core_machine_fdc_refresh(fdc);
                failed |= (core_machine_port_read(port, fdc_config.direction_port) & VFDC_DIR_DC) == 0u;

                /* MFM data service exposes one byte, withdraws DRQ, and cannot
                   expose the next byte before the 15-us (120-tick) gate. */
                fixture.read_count = 0u;
                core_machine_port_write(port, fdc_config.control_port, 0u);
                core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
                core_machine_fdc_write_dma2(port, 0x0600u, 1u);
                core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                core_machine_fdc_advance_at(fdc, 100u);
                failed |= !core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                    &machine->shared_dma_secondary);
                test_dma_transfers(&machine->shared_dma_latch,
                    &machine->shared_dma_primary, &machine->shared_dma_secondary,
                    &machine->executor_memory, &machine->executor_port, 1u);
                failed |= fixture.read_count != 1u ||
                    core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                        &machine->shared_dma_secondary);
                core_machine_fdc_advance_at(fdc, 219u);
                failed |= core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                    &machine->shared_dma_secondary);
                core_machine_fdc_advance_at(fdc, 220u);
                failed |= !core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                    &machine->shared_dma_secondary);
                test_dma_transfers(&machine->shared_dma_latch,
                    &machine->shared_dma_primary, &machine->shared_dma_secondary,
                    &machine->executor_memory, &machine->executor_port, 1u);
                failed |= fixture.read_count != 2u || fdc->data.phase !=
                    core_machine_fdc_PHASE_PENDING_COMPLETE || fdc->connect.irq_source.asserted ||
                    fdc->data.dma_byte_gate_pending || fdc->data.next_dma_byte_tick != 0u;
                core_machine_fdc_advance_at(fdc, 229u);
                failed |= !fdc->connect.irq_source.asserted;
                result[0] = (lib_u8)core_machine_port_read(port,
                    fdc_config.data_port);
                failed |= fdc->connect.irq_source.asserted || fdc->data.flagINTR ||
                    result[0] != core_machine_fdc_ST0_NORMAL;
                for (lib_u8 result_index = 1u; result_index < sizeof(result);
                    ++result_index) {
                    result[result_index] = (lib_u8)core_machine_port_read(port,
                        fdc_config.data_port);
                }
                failed |= (core_machine_port_read(port, fdc_config.status_port) &
                    (VFDC_MSR_CB | VFDC_MSR_DIO)) != 0u;
                core_machine_port_write(port, fdc_config.dor_port, 0u);
                failed |= core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                    &machine->shared_dma_secondary) || fdc->data.dma_byte_gate_pending;

                /* Scan consumes guest comparison bytes through DMA2's
                   memory-to-device direction, then reports through the same
                   seven-byte IRQ result phase. */
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
                core_machine_port_write(port, fdc_config.control_port, 0u);
                lib_memory_set(scan_dma, 0xa5u, sizeof(scan_dma));
                failed |= core_machine_memory_write_physical(&machine->executor_memory,
                    0x0600u, (lib_uptr)scan_dma, sizeof(scan_dma)) != LIB_STATUS_OK;
                core_machine_fdc_write_dma2(port, 0x0600u, 511u);
                core_machine_port_write(port, 0x000bu, 0x4au);
                core_machine_fdc_command(fdc, port, scan_equal, sizeof(scan_equal));
                for (lib_u32 index = 0u; index < sizeof(scan_dma); ++index) {
                    test_dma_transfers(&machine->shared_dma_latch,
                        &machine->shared_dma_primary, &machine->shared_dma_secondary,
                        &machine->executor_memory, &machine->executor_port, 1u);
                    if (index + 1u < sizeof(scan_dma)) {
                        core_machine_fdc_advance_at(fdc,
                        fdc->data.elapsed_ticks + 8u * 31u);
                    }
                }
                failed |= fdc->data.phase != core_machine_fdc_PHASE_PENDING_COMPLETE ||
                    fdc->data.dma_byte_gate_pending;
                core_machine_fdc_advance(fdc);
                failed |= !fdc->connect.irq_source.asserted ||
                    !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (VFDC_ST2_SCAN_MATCH | VFDC_ST2_SCAN_MISMATCH)) !=
                        VFDC_ST2_SCAN_MATCH;

                /* Intel 8272A transfers until DMA asserts TC.  EOT is the
                   controller's sector-search limit, not an upfront medium
                   geometry rejection: a one-sector DMA request remains
                   successful even when firmware's provisional EOT exceeds
                   the inserted medium's last sector. */
                fixture.read_count = 0u;
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
                core_machine_port_write(port, fdc_config.control_port,
                    VFDC_CCR_RATE_300);
                core_machine_fdc_write_dma2(port, 0x0600u, 511u);
                core_machine_fdc_command(fdc, port, read_sector_dma_terminal,
                    sizeof(read_sector_dma_terminal));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    test_dma_transfers(&machine->shared_dma_latch,
                        &machine->shared_dma_primary, &machine->shared_dma_secondary,
                        &machine->executor_memory, &machine->executor_port, 1u);
                    if (index + 1u < 512u) core_machine_fdc_advance_at(fdc,
                        fdc->data.elapsed_ticks + 8u * 25u);
                }
                core_machine_fdc_advance(fdc);
                failed |= fixture.read_count != 512u ||
                    !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != core_machine_fdc_ST0_NORMAL;
                core_machine_port_write(port, fdc_config.dor_port, 0u);
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_advance_at(fdc, fdc->data.reset_due_tick);
                for (lib_u8 reset_drive = 0u;
                    reset_drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++reset_drive) {
                    core_machine_fdc_command(fdc, port,
                        (const lib_u8[]){0x08u}, 1u);
                    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                        result[0] != (core_machine_fdc_ST0_READY_CHANGE | reset_drive) ||
                        result[1] != 0u || fdc->connect.irq_source.asserted;
                }
                core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));
                core_machine_port_write(port, fdc_config.control_port, VFDC_CCR_RATE_250);

                fixture.forced_read_result = CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
                core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                (void)core_machine_port_read(port, fdc_config.data_port);
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[1] & 0x04u) == 0u;
                fixture.forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK;
                fixture.read_only = LIB_TRUE;
                core_machine_fdc_command(fdc, port, write_sector, sizeof(write_sector));
                core_machine_port_write(port, fdc_config.data_port, 0x33u);
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[1] & 0x02u) == 0u;
                fixture.read_only = LIB_FALSE;
                fixture.present = LIB_FALSE;
                core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[1] & 0x04u) == 0u;

                /* The same MFM byte-service interval applies when the host
                   services 3F5h directly rather than through DMA2. */
                fixture.present = LIB_TRUE;
                fixture.read_count = 0u;
                core_machine_port_write(port, fdc_config.control_port, 0u);
                core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));
                core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                core_machine_fdc_advance_at(fdc, fdc->data.elapsed_ticks + 1u);
                failed |= core_machine_port_read(port, fdc_config.data_port) != 0xa5u ||
                    fixture.read_count != 1u || !fdc->data.ndma_byte_gate_pending;
                ndma_gate_tick = fdc->data.next_ndma_byte_tick;
                failed |= ndma_gate_tick != fdc->data.elapsed_ticks +
                    8u * 15u;
                core_machine_fdc_advance_at(fdc, ndma_gate_tick - 1u);
                failed |= (core_machine_port_read(port, fdc_config.status_port) & VFDC_MSR_RQM) != 0u ||
                    fixture.read_count != 1u;
                core_machine_fdc_advance_at(fdc, ndma_gate_tick);
                failed |= (core_machine_port_read(port, fdc_config.status_port) &
                    VFDC_MSR_ProcessRead) != VFDC_MSR_ProcessRead ||
                    fdc->data.ndma_byte_gate_pending;
                (void)core_machine_port_read(port, fdc_config.data_port);
                failed |= fixture.read_count != 2u;
                core_machine_port_write(port, fdc_config.dor_port, 0u);
                failed |= fdc->data.ndma_byte_gate_pending;

                /* Scan is host-to-controller execution too.  This FM command
                   uses the 31-us byte gate, and DOR reset cancels it. */
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_advance_at(fdc, fdc->data.reset_due_tick);
                for (lib_u8 reset_drive = 0u;
                    reset_drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++reset_drive) {
                    core_machine_fdc_command(fdc, port,
                        (const lib_u8[]){0x08u}, 1u);
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
                    8u * 31u;
                core_machine_port_write(port, fdc_config.dor_port, 0u);
                failed |= fdc->data.phase != core_machine_fdc_PHASE_COMMAND ||
                    fdc->data.ndma_byte_gate_pending;

                /* An unqualified service-time conversion is still a complete
                   logical DRQ/DACK handshake: single-mode DMA may consume
                   successive bytes without a fabricated delay. */
                fdc->connect.config.clock_ticks_per_second = 0u;
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
                core_machine_fdc_write_dma2(port, 0x0600u, 1u);
                fallback_read_count = fixture.read_count;
                core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                test_dma_transfers(&machine->shared_dma_latch,
                    &machine->shared_dma_primary, &machine->shared_dma_secondary,
                    &machine->executor_memory, &machine->executor_port, 1u);
                failed |= fixture.read_count != fallback_read_count + 1u ||
                    !fdc->data.dma_byte_gate_pending ||
                    core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                        &machine->shared_dma_secondary);
                core_machine_fdc_advance_at(fdc, fdc->data.next_dma_byte_tick);
                failed |= !core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                    &machine->shared_dma_secondary);
                test_dma_transfers(&machine->shared_dma_latch,
                    &machine->shared_dma_primary, &machine->shared_dma_secondary,
                    &machine->executor_memory, &machine->executor_port, 1u);
                failed |= fixture.read_count != fallback_read_count + 2u || fdc->data.phase !=
                    core_machine_fdc_PHASE_PENDING_COMPLETE;
                core_machine_fdc_advance(fdc);
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != core_machine_fdc_ST0_NORMAL || result[1] != 0u ||
                    result[2] != 0u;
            }
        }
    }
    if (!failed) failed |= core_machine_fdc_readiness_matrix(machine, &fixture);
    if (!failed) failed |= core_machine_fdc_ready_edges(fdc, port, &fixture);
    if (!failed) failed |= core_machine_fdc_result_identity(fdc, port, &fixture);
    if (!failed) failed |= core_machine_fdc_scan_status(fdc, port, &fixture);
    if (!failed) failed |= core_machine_fdc_scan_sequence(fdc, port, &fixture);
    if (!failed) failed |= core_machine_fdc_read_sequence(fdc, port, &fixture);
    if (!failed) failed |= core_machine_fdc_write_terminal(machine, &fixture);
    if (!failed) failed |= core_machine_fdc_terminal_id(machine, &fixture);
    if (!failed) failed |= core_machine_fdc_seek_ownership(fdc, port);
    if (!failed) failed |= core_machine_fdc_no_implied_seek(fdc, port, &fixture);
    if (!failed) failed |= core_machine_fdc_seek_pins(fdc, port);
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    if (failed) {
        fprintf(stderr, "M5:T376:S4:8272A-SCAN:FAIL %x\n", failed);
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
    puts("M5:T539:S12:FDC-readiness-matrix-and-edges:OK");
    puts("M5:T539:S11:FDC-seek-ownership:OK");
    puts("M5:T539:S12:FDC-scan-status:OK");
    puts("M5:T539:S12:FDC-ST0-identity:OK");
    puts("M5:T539:S12:FDC-PCN-Track0-READY:OK");
    return 0;
}
