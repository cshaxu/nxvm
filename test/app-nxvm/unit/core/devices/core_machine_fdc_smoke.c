#include "support/dma_fixture.h"
#include "support/fdc_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/dma_bus.h"
#include "app-nxvm/devices/fdc.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_board_state.h"
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
    test_fdc_advance(fdc);
}

static lib_bool core_machine_fdc_command(core_machine_fdc *fdc, t_port *port,
    const lib_u8 *bytes, lib_size count)
{
    core_machine_fdc_submit(fdc, port, bytes, count);
    return test_fdc_finish_seeks(fdc);
}

/* A failed observation is a fixture failure, never an all-zero success. */
static x86_fdc_observation observe(core_machine_fdc *fdc)
{
    x86_fdc_observation value;
    if (x86_fdc_capture(fdc->chip, &value) != LIB_STATUS_OK) {
        fputs("FDC observation failed\n", stderr);
        exit(1);
    }
    return value;
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

    test_fdc_advance(fdc);
    for (index = 0u; index < count; ++index) {
        if ((core_machine_port_read(port, 0x03f4u) & TEST_FDC_MSR_READY_READ) !=
            TEST_FDC_MSR_READY_READ) return LIB_FALSE;
        result[index] = (lib_u8)core_machine_port_read(port, 0x03f5u);
    }
    return (core_machine_port_read(port, 0x03f4u) & (TEST_FDC_MSR_CB | TEST_FDC_MSR_DIO)) == 0u;
}

/* S12 supersedes the S10 characterization: READY is a pin, whereas absent
 * media, motor and selection prevent a record from being delivered. */
static lib_i32 core_machine_fdc_readiness_matrix(core_machine *machine,
    core_machine_fdc_fixture_media *media)
{
    core_machine_fdc *fdc = &machine->board->fdc;
    t_port *port = &machine->executor_port;
    static const struct {
        lib_u8 bytes[9];
        lib_u8 count;
        lib_u8 msr;
    } commands[] = {
        {{0x46u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu}, 9u,
            TEST_FDC_MSR_CB},
        {{0x4cu, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu}, 9u,
            TEST_FDC_MSR_CB},
        {{0x45u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu}, 9u,
            TEST_FDC_MSR_CB},
        {{0x49u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu}, 9u,
            TEST_FDC_MSR_CB},
        {{0x51u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 1u}, 9u,
            TEST_FDC_MSR_CB},
        {{0x59u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 1u}, 9u,
            TEST_FDC_MSR_CB},
        {{0x5du, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 1u}, 9u,
            TEST_FDC_MSR_CB},
        {{0x42u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu}, 9u,
            TEST_FDC_MSR_CB},
        {{0x4du, 0u, 2u, 1u, 0x1bu, 0xa5u}, 6u,
            TEST_FDC_MSR_CB},
        {{0x4au, 0u}, 2u, TEST_FDC_MSR_RESULT}
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
            lib_u8 result[7] = {0};
            lib_bool mismatch;

            if (during_transfer && commands[command].msr == TEST_FDC_MSR_RESULT)
                continue;
            core_machine_fdc_reset(fdc);
            fdc->connect.config.ready_mask = inputs[initial].ready;
            fdc->connect.drives.installed_mask = inputs[initial].installed;
            media->present = inputs[initial].present;
            media->marks[0] = (commands[command].bytes[0] & 0x1fu) == 0x0cu ?
                CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA :
                CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
            core_machine_port_write(port, 0x03f2u, inputs[initial].dor);
            failed |= !core_machine_fdc_command(fdc, port,
                (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
            core_machine_fdc_write_dma2(port, 0x0600u, 511u);
            failed |= !core_machine_fdc_command(fdc, port, commands[command].bytes,
                commands[command].count);
            if (during_transfer) {
                failed |= core_machine_port_read(port, 0x03f4u) != commands[command].msr;
                fdc->connect.config.ready_mask = inputs[input].ready;
                fdc->connect.drives.installed_mask = inputs[input].installed;
                media->present = inputs[input].present;
                core_machine_port_write(port, 0x03f2u, inputs[input].dor);
                test_fdc_advance(fdc);
            }
            if (!inputs[input].admitted) {
                const lib_bool not_ready = inputs[input].ready == 0u;
                test_fdc_advance(fdc);
                mismatch = !fdc->connect.irq_source.asserted ||
                    !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != (not_ready ? 0x48u : 0x40u) ||
                    result[1] != (not_ready ? 0u : 0x04u) ||
                    result[2] != 0u || fdc->connect.irq_source.asserted;
            } else {
                mismatch = core_machine_port_read(port, 0x03f4u) != commands[command].msr ||
                    fdc->connect.irq_source.asserted !=
                        (commands[command].msr == TEST_FDC_MSR_RESULT);
            }
            mismatch |= core_machine_dma_has_pending_request(
                &machine->board->shared_dma_primary, &machine->board->shared_dma_secondary) !=
                (inputs[input].admitted &&
                    commands[command].msr != TEST_FDC_MSR_RESULT);
            if (mismatch) {
                fprintf(stderr, "FDC readiness variant=%zu command=%02x st=%02x/%02x/%02x irq=%u msr=%02x dma=%u\n",
                    variant, commands[command].bytes[0], result[0], result[1], result[2],
                    fdc->connect.irq_source.asserted,
                    core_machine_port_read(port, 0x03f4u),
                    core_machine_dma_has_pending_request(&machine->board->shared_dma_primary,
                        &machine->board->shared_dma_secondary));
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
        failed |= !core_machine_fdc_command(fdc, port,
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


static lib_i32 core_machine_fdc_write_terminal(core_machine *machine,
    core_machine_fdc_fixture_media *media)
{
    core_machine_fdc *fdc = &machine->board->fdc;
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
                failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x03u, 0xdfu, 2u}, 3u);
                core_machine_port_write(port, 0x03f7u, VFDC_CCR_RATE_500);
                core_machine_fdc_write_dma2(port, 0x0600u, length - 1u);
                core_machine_port_write(port, 0x000bu, 0x4au);
                failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){
                    deleted ? 0x49u : 0x45u, 0u, 0u, 0u, 1u, 2u, 3u, 0x1bu, 0xffu}, 9u);
                for (lib_u16 byte = 0u; byte < length; ++byte) {
                    test_dma_transfers(&machine->board->shared_dma_latch,
                        &machine->board->shared_dma_primary, &machine->board->shared_dma_secondary,
                        machine, port, 1u);
                    if (byte + 1u < length)
                        core_machine_fdc_advance_at(fdc, observe(fdc).next_dma_byte_tick);
                }
                failed |= media->write_count - writes != length;
                if (cases[index].action != FINISH) {
                    const lib_u64 due = observe(fdc).next_dma_byte_tick;
                    if (cases[index].action == RESET)
                        core_machine_port_write(port, 0x03f2u, 0u);
                    else if (cases[index].action == MOTOR_OFF)
                        core_machine_port_write(port, 0x03f2u, 0x0cu);
                    else media->forced_write_result = CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
                    core_machine_fdc_advance_at(fdc, due);
                    if (cases[index].action == RESET)
                        failed |= (core_machine_port_read(port, 0x03f4u) & TEST_FDC_MSR_CB) != 0u ||
                            fdc->connect.irq_source.asserted;
                    else failed |= !core_machine_fdc_read_result(fdc, port, result, 7u) ||
                        result[0] != 0x40u || result[1] !=
                            (cases[index].action == MOTOR_OFF ? 0x04u : 0x02u);
                    failed |= media->write_count - writes != length ||
                        observe(fdc).dma_byte_gate_pending ||
                        core_machine_dma_has_pending_request(&machine->board->shared_dma_primary,
                            &machine->board->shared_dma_secondary);
                    media->forced_write_result = CORE_MACHINE_MEDIA_RESULT_OK;
                    for (lib_u16 byte = 0u; byte < sizeof(media->bytes); ++byte)
                        failed |= media->bytes[byte] != (byte < length ? 0xa5u : 0x5au);
                    if (failed) fprintf(stderr, "FDC WRITE TC interruption=%u\n", cases[index].action);
                    continue;
                }
                for (lib_u16 byte = length; byte < 512u; ++byte) {
                    const lib_u64 due = observe(fdc).next_dma_byte_tick;
                    const lib_u32 before = media->write_count;
                    failed |= core_machine_port_read(port, 0x03f4u) != TEST_FDC_MSR_CB ||
                        core_machine_dma_has_pending_request(&machine->board->shared_dma_primary,
                            &machine->board->shared_dma_secondary) ||
                        (core_machine_port_read(port, 0x03f4u) & TEST_FDC_MSR_RQM) != 0u;
                    core_machine_fdc_advance_at(fdc, due - 1u);
                    failed |= media->write_count != before;
                    core_machine_fdc_advance_at(fdc, due);
                    failed |= media->write_count != before + 1u;
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, 7u) ||
                    result[0] != 0u || result[1] != 0u || result[2] != 0u ||
                    media->write_count - writes != 512u || observe(fdc).dma_byte_gate_pending ||
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
    core_machine_fdc *fdc = &machine->board->fdc;
    t_port *port = &machine->executor_port;
    const core_machine_fdc_config saved_config = fdc->connect.config;
    lib_i32 failed = 0;

    fdc->connect.config.clock_ticks_per_second = 0u;
    core_machine_port_write(port, 0x03f7u, VFDC_CCR_RATE_500);
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
            failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x03u, 0xdfu, 2u}, 3u);
            core_machine_fdc_write_dma2(port, 0x0600u, length - 1u);
            if (command >= 2u) core_machine_port_write(port, 0x000bu, 0x4au);
            failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){
                commands[command] | (multi_track ? 0x80u : 0u), head << 2u,
                0u, head, 1u, 2u, at_eot ? 1u : 2u, 0x1bu, 0xffu}, 9u);
            for (lib_u16 byte = 0u; byte < length; ++byte) {
                test_dma_transfers(&machine->board->shared_dma_latch,
                    &machine->board->shared_dma_primary, &machine->board->shared_dma_secondary,
                    machine, port, 1u);
                if (byte + 1u < length)
                    core_machine_fdc_advance_at(fdc, observe(fdc).next_dma_byte_tick);
            }
            for (lib_u16 byte = length; byte < 512u &&
                command >= 2u; ++byte)
                core_machine_fdc_advance_at(fdc, observe(fdc).next_dma_byte_tick);
            if (!core_machine_fdc_read_result(fdc, port, result, 7u) ||
                (result[0] & 0xc0u) != 0u || result[1] != 0u || result[2] != 0u ||
                result[3] != (at_eot && (!multi_track || head != 0u) ? 1u : 0u) ||
                result[4] != (at_eot && multi_track ? head ^ 1u : head) ||
                result[5] != (at_eot ? 1u : 2u) || result[6] != 2u ||
                fdc->drive_cylinder[0] != 0u || observe(fdc).pcn[0] != 0u) {
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
        failed |= !core_machine_fdc_command(fdc, port,
            (const lib_u8[]){commands[index], 0u, 1u, 0u, 1u, 2u, 1u, 0x1bu, 1u}, 9u);
        failed |= !core_machine_fdc_read_result(fdc, port, result, 7u) ||
            result[0] != 0x40u || result[1] != 0x04u ||
            result[2] != (commands[index] == 0x42u ? 0u : 0x10u) ||
            result[3] != (commands[index] == 0x42u ? 1u : 0u) ||
            fdc->drive_cylinder[0] != 0u || observe(fdc).pcn[0] != 0u;
    }
    failed |= media->read_count != reads || media->write_count != writes;
    /* READ ID must ignore the previous command's requested cylinder. */
    failed |= observe(fdc).cylinder != 1u;
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x4au, 0u}, 2u);
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
    core_machine_port_write(port, 0x03f7u, VFDC_CCR_RATE_500);
    fdc->drive_cylinder[0] = 0u;
    core_machine_fdc_reset(fdc);
    core_machine_port_write(port, 0x03f2u, 0x1cu);
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x0fu, 0u, 100u}, 3u);
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x20u || result[1] != 100u || fdc->drive_cylinder[0] != 39u;

    /* Reset changes PCN, not the drive's physical position or Track0. */
    core_machine_fdc_reset(fdc);
    failed |= observe(fdc).pcn[0] != 0u || fdc->drive_cylinder[0] != 39u;
    core_machine_port_write(port, 0x03f2u, 0x1cu);
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x04u, 0u}, 2u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 1u) || (result[0] & 0x10u);
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x07u, 0u}, 2u);
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x20u || result[1] != 0u ||
        fdc->drive_cylinder[0] != 0u;

    /* No Track0 within 77 pulses is EC; a second command can finish. */
    fdc->connect.drives.cylinder_count[0] = 80u;
    fdc->drive_cylinder[0] = 79u;
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x07u, 0u}, 2u);
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x70u || result[1] != 0u ||
        fdc->drive_cylinder[0] != 2u;
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x07u, 0u}, 2u);
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x20u || fdc->drive_cylinder[0] != 0u;

    /* READY loss aborts at the current PCN, not at the requested target. */
    core_machine_fdc_submit(fdc, port, (const lib_u8[]){0x0fu, 0u, 5u}, 3u);
    failed |= !test_fdc_advance_due(fdc);
    failed |= !test_fdc_advance_due(fdc);
    fdc->connect.config.ready_mask = 0x0eu;
    test_fdc_advance(fdc);
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x68u || result[1] != 2u || fdc->drive_cylinder[0] != 2u;
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x0fu, 4u, 9u}, 3u);
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x6cu || result[1] != 2u || fdc->drive_cylinder[0] != 2u;

    /* DOR routes STEP to mechanism zero even when the chip counts unit one. */
    fdc->connect.config.ready_mask = 0x0fu;
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x0fu, 5u, 1u}, 3u);
    failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
        result[0] != 0x25u || result[1] != 1u || fdc->drive_cylinder[0] != 3u;
    fdc->connect.drives = saved_drives;
    fdc->connect.config = saved_config;
    core_machine_fdc_reset(fdc);
    if (failed) fputs("FDC PCN/Track0/READY qualification failed\n", stderr);
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
        .fpu_profile = X86_FPU_PROFILE_NONE,
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
        fdc = &machine->board->fdc;
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
                    !observe(fdc).reset_pending || observe(fdc).reset_due_tick != 8192u;
                core_machine_fdc_advance_at(fdc, 8191u);
                failed |= fdc->connect.irq_source.asserted;
                core_machine_fdc_advance_at(fdc, 8192u);
                failed |= !fdc->connect.irq_source.asserted;
                for (lib_u8 reset_drive = 0u;
                    reset_drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++reset_drive) {
                    failed |= !core_machine_fdc_command(fdc, port,
                        (const lib_u8[]){0x08u}, 1u);
                    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                        result[0] != (TEST_FDC_ST0_READY_CHANGE | reset_drive) ||
                        result[1] != 0u || fdc->connect.irq_source.asserted;
                }
                failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
                failed |= !core_machine_fdc_read_result(fdc, port, result, 1u) ||
                    result[0] != 0x80u || fdc->connect.irq_source.asserted;
                /* A new Recalibrate supersedes an undrained reset notice;
                   Sense Interrupt must report the completed operation. */
                core_machine_port_write(port, fdc_config.dor_port, 0u);
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_advance_at(fdc, observe(fdc).reset_due_tick);
                failed |= !core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x07u, 0u}, 2u);
                test_fdc_advance(fdc);
                failed |= !core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x08u}, 1u);
                failed |= observe(fdc).reset_sense_mask != 0u ||
                    !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                    result[0] != TEST_FDC_ST0_SEEK_END || result[1] != 0u;
                failed |= !core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));
                core_machine_port_write(port, fdc_config.control_port, VFDC_CCR_RATE_250);

                failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x10u}, 1u);
                failed |= fdc->connect.irq_source.asserted ||
                    !core_machine_fdc_read_result(fdc, port, result, 1u) ||
                    result[0] != 0x80u || (core_machine_port_read(port, 0x03f4u) & TEST_FDC_MSR_CB) != 0u;

                /* Chip-local tests own SPECIFY retention; this checks DOR
                   wiring and all four post-reset SIS results through ports. */
                core_machine_port_write(port, fdc_config.dor_port, 0x18u);
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_advance_at(fdc, observe(fdc).reset_due_tick);
                for (lib_u8 reset_drive = 0u;
                    reset_drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++reset_drive) {
                    failed |= !core_machine_fdc_command(fdc, port,
                        (const lib_u8[]){0x08u}, 1u);
                    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                        result[0] != (TEST_FDC_ST0_READY_CHANGE | reset_drive) ||
                        result[1] != 0u || fdc->connect.irq_source.asserted;
                }
                failed |= !core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));


                /* An installed empty drive can seek: media availability
                   controls sector transfer, not the mechanical completion. */
                fdc->connect.drives.installed_mask |= 0x02u;
                core_machine_port_write(port, fdc_config.dor_port, 0x2du);
                failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x07u, 1u}, 2u);
                failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
                failed |= !core_machine_fdc_read_result(fdc, port, result, 2u);
                failed |= !core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x0fu, 0x01u, 0x01u}, 3u);
                failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
                if (!core_machine_fdc_read_result(fdc, port, result, 2u) ||
                    result[0] != (TEST_FDC_ST0_NORMAL |
                    TEST_FDC_ST0_SEEK_END | 1u) || result[1] != 1u) failed |= 0x100;
                /* ST3 observes the selected drive's Track-0 input, not the
                   controller's most recently completed seek. */
                fdc->drive_cylinder[0u] = 0u;
                core_machine_port_write(port, fdc_config.dor_port, 0x2du);
                failed |= !core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x04u, 0x01u}, 2u);
                if (!core_machine_fdc_read_result(fdc, port, result, 1u) ||
                    result[0] != 0x21u) failed |= 0x200;
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);

                for (lib_u32 index = 0u; index < sizeof(read_sector); ++index) {
                    core_machine_port_write(port, fdc_config.data_port, read_sector[index]);
                }
                failed |= core_machine_port_read(port, fdc_config.status_port) != TEST_FDC_MSR_CB ||
                    fdc->connect.irq_source.asserted;
                test_fdc_advance(fdc);
                failed |= (core_machine_port_read(port, 0x03f4u) & TEST_FDC_MSR_PROCESS_READ) != TEST_FDC_MSR_PROCESS_READ;
                failed |= core_machine_port_read(port, fdc_config.data_port) != 0x4au;
                for (lib_u32 index = 1u; index < 512u; ++index) {
                    (void)core_machine_port_read(port, fdc_config.data_port);
                }
                failed |= core_machine_port_read(port, fdc_config.status_port) != TEST_FDC_MSR_CB ||
                    fdc->connect.irq_source.asserted;
                test_fdc_advance(fdc);
                failed |= core_machine_port_read(port, 0x03f4u) != TEST_FDC_MSR_RESULT ||
                    !fdc->connect.irq_source.asserted ||
                    !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != TEST_FDC_ST0_NORMAL ||
                    fdc->connect.irq_source.asserted;
                fixture.read_count = 0u;

                failed |= !core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                failed |= core_machine_port_read(port, fdc_config.data_port) != 0x4au;
                for (lib_u32 index = 1u; index < 512u; ++index) {
                    (void)core_machine_port_read(port, fdc_config.data_port);
                }
                test_fdc_advance(fdc);
                failed |= !fdc->connect.irq_source.asserted ||
                    !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != TEST_FDC_ST0_NORMAL || fixture.read_count != 512u ||
                    fdc->connect.irq_source.asserted;

                fixture.marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA;
                failed |= !core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    (void)core_machine_port_read(port, fdc_config.data_port);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & TEST_FDC_ST2_CONTROL_MARK) == 0u;
                failed |= !core_machine_fdc_command(fdc, port, read_deleted_sector,
                    sizeof(read_deleted_sector));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    (void)core_machine_port_read(port, fdc_config.data_port);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & TEST_FDC_ST2_CONTROL_MARK) != 0u;

                fixture.marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
                failed |= !core_machine_fdc_command(fdc, port, write_deleted_sector,
                    sizeof(write_deleted_sector));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x6bu : 0u);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != TEST_FDC_ST0_NORMAL ||
                    fixture.marks[0] != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA ||
                    fixture.bytes[0] != 0x6bu;

                fixture.write_count = 0u;
                failed |= !core_machine_fdc_command(fdc, port, write_sector, sizeof(write_sector));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x5au : 0u);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != TEST_FDC_ST0_NORMAL || fixture.write_count != 512u ||
                    fixture.bytes[0] != 0x5au;

                /* Scan commands receive comparison bytes through the same
                   host-to-controller path as a write, but never mutate media.
                   FFh is the documented no-care compare byte. */
                fixture.marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
                failed |= !core_machine_fdc_command(fdc, port, scan_equal, sizeof(scan_equal));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x5au : 0xffu);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (TEST_FDC_ST2_SCAN_MATCH | TEST_FDC_ST2_SCAN_MISMATCH)) !=
                        TEST_FDC_ST2_SCAN_MATCH || fixture.bytes[0] != 0x5au;
                failed |= !core_machine_fdc_command(fdc, port, scan_low_or_equal,
                    sizeof(scan_low_or_equal));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x60u : 0xffu);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (TEST_FDC_ST2_SCAN_MATCH | TEST_FDC_ST2_SCAN_MISMATCH)) !=
                        0u;
                failed |= !core_machine_fdc_command(fdc, port, scan_high_or_equal,
                    sizeof(scan_high_or_equal));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x50u : 0xffu);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (TEST_FDC_ST2_SCAN_MATCH | TEST_FDC_ST2_SCAN_MISMATCH)) !=
                        0u;
                failed |= !core_machine_fdc_command(fdc, port, scan_equal, sizeof(scan_equal));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x50u : 0xffu);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (TEST_FDC_ST2_SCAN_MATCH | TEST_FDC_ST2_SCAN_MISMATCH)) !=
                        TEST_FDC_ST2_SCAN_MISMATCH;
                fixture.marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA;
                failed |= !core_machine_fdc_command(fdc, port, scan_equal, sizeof(scan_equal));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    core_machine_port_write(port, fdc_config.data_port,
                        index == 0u ? 0x5au : 0xffu);
                }
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (TEST_FDC_ST2_SCAN_MATCH | TEST_FDC_ST2_CONTROL_MARK)) !=
                        (TEST_FDC_ST2_SCAN_MATCH | TEST_FDC_ST2_CONTROL_MARK);
                failed |= !core_machine_fdc_command(fdc, port, scan_equal_skip,
                    sizeof(scan_equal_skip));
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (TEST_FDC_ST2_SCAN_MATCH | TEST_FDC_ST2_SCAN_MISMATCH |
                    TEST_FDC_ST2_CONTROL_MARK)) != (TEST_FDC_ST2_SCAN_MISMATCH | TEST_FDC_ST2_CONTROL_MARK);
                fixture.marks[0] = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;

                failed |= !core_machine_fdc_command(fdc, port, format_track, sizeof(format_track));
                failed |= !core_machine_fdc_command(fdc, port, format_id, sizeof(format_id));
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != TEST_FDC_ST0_NORMAL || fixture.format_count != 1u ||
                    fixture.generation != 2u || fixture.bytes[511] != 0xa5u;

                /* A real STEP acknowledges media change; SEEK to the existing
                   PCN produces no pulse and cannot clear the drive's latch. */
                failed |= !core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x0fu, 0x00u, 0x01u}, 3u);
                failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
                failed |= !core_machine_fdc_read_result(fdc, port, result, 2u);
                failed |= !core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x0fu, 0x00u, 0x00u}, 3u);
                failed |= !core_machine_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
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
                failed |= !core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
                core_machine_fdc_write_dma2(port, 0x0600u, 1u);
                failed |= !core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                core_machine_fdc_advance_at(fdc, 100u);
                failed |= !core_machine_dma_has_pending_request(&machine->board->shared_dma_primary,
                    &machine->board->shared_dma_secondary);
                test_dma_transfers(&machine->board->shared_dma_latch,
                    &machine->board->shared_dma_primary, &machine->board->shared_dma_secondary,
                    machine, &machine->executor_port, 1u);
                failed |= fixture.read_count != 1u ||
                    core_machine_dma_has_pending_request(&machine->board->shared_dma_primary,
                        &machine->board->shared_dma_secondary);
                core_machine_fdc_advance_at(fdc, 219u);
                failed |= core_machine_dma_has_pending_request(&machine->board->shared_dma_primary,
                    &machine->board->shared_dma_secondary);
                core_machine_fdc_advance_at(fdc, 220u);
                failed |= !core_machine_dma_has_pending_request(&machine->board->shared_dma_primary,
                    &machine->board->shared_dma_secondary);
                test_dma_transfers(&machine->board->shared_dma_latch,
                    &machine->board->shared_dma_primary, &machine->board->shared_dma_secondary,
                    machine, &machine->executor_port, 1u);
                failed |= fixture.read_count != 2u || core_machine_port_read(port, 0x03f4u) != TEST_FDC_MSR_CB || fdc->connect.irq_source.asserted ||
                    observe(fdc).dma_byte_gate_pending || observe(fdc).next_dma_byte_tick != 0u;
                core_machine_fdc_advance_at(fdc, 229u);
                failed |= !fdc->connect.irq_source.asserted;
                result[0] = (lib_u8)core_machine_port_read(port,
                    fdc_config.data_port);
                failed |= fdc->connect.irq_source.asserted || observe(fdc).interrupt_pending ||
                    result[0] != TEST_FDC_ST0_NORMAL;
                for (lib_u8 result_index = 1u; result_index < sizeof(result);
                    ++result_index) {
                    result[result_index] = (lib_u8)core_machine_port_read(port,
                        fdc_config.data_port);
                }
                failed |= (core_machine_port_read(port, fdc_config.status_port) &
                    (TEST_FDC_MSR_CB | TEST_FDC_MSR_DIO)) != 0u;
                core_machine_port_write(port, fdc_config.dor_port, 0u);
                failed |= core_machine_dma_has_pending_request(&machine->board->shared_dma_primary,
                    &machine->board->shared_dma_secondary) || observe(fdc).dma_byte_gate_pending;

                /* Scan consumes guest comparison bytes through DMA2's
                   memory-to-device direction, then reports through the same
                   seven-byte IRQ result phase. */
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                failed |= !core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
                core_machine_port_write(port, fdc_config.control_port, 0u);
                lib_memory_set(scan_dma, 0xa5u, sizeof(scan_dma));
                failed |= core_machine_memory_write_physical(&machine->executor_memory,
                    0x0600u, (lib_uptr)scan_dma, sizeof(scan_dma)) != LIB_STATUS_OK;
                core_machine_fdc_write_dma2(port, 0x0600u, 511u);
                core_machine_port_write(port, 0x000bu, 0x4au);
                failed |= !core_machine_fdc_command(fdc, port, scan_equal, sizeof(scan_equal));
                for (lib_u32 index = 0u; index < sizeof(scan_dma); ++index) {
                    test_dma_transfers(&machine->board->shared_dma_latch,
                        &machine->board->shared_dma_primary, &machine->board->shared_dma_secondary,
                        machine, &machine->executor_port, 1u);
                    if (index + 1u < sizeof(scan_dma)) {
                        core_machine_fdc_advance_at(fdc,
                        observe(fdc).elapsed_ticks + 8u * 31u);
                    }
                }
                failed |= observe(fdc).dma_byte_gate_pending;
                test_fdc_advance(fdc);
                failed |= !fdc->connect.irq_source.asserted ||
                    !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[2] & (TEST_FDC_ST2_SCAN_MATCH | TEST_FDC_ST2_SCAN_MISMATCH)) !=
                        TEST_FDC_ST2_SCAN_MATCH;

                /* Intel 8272A transfers until DMA asserts TC.  EOT is the
                   controller's sector-search limit, not an upfront medium
                   geometry rejection: a one-sector DMA request remains
                   successful even when firmware's provisional EOT exceeds
                   the inserted medium's last sector. */
                fixture.read_count = 0u;
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                failed |= !core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
                core_machine_port_write(port, fdc_config.control_port,
                    VFDC_CCR_RATE_300);
                core_machine_fdc_write_dma2(port, 0x0600u, 511u);
                failed |= !core_machine_fdc_command(fdc, port, read_sector_dma_terminal,
                    sizeof(read_sector_dma_terminal));
                for (lib_u32 index = 0u; index < 512u; ++index) {
                    test_dma_transfers(&machine->board->shared_dma_latch,
                        &machine->board->shared_dma_primary, &machine->board->shared_dma_secondary,
                        machine, &machine->executor_port, 1u);
                    if (index + 1u < 512u) core_machine_fdc_advance_at(fdc,
                        observe(fdc).elapsed_ticks + 8u * 25u);
                }
                test_fdc_advance(fdc);
                failed |= fixture.read_count != 512u ||
                    !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != TEST_FDC_ST0_NORMAL;
                core_machine_port_write(port, fdc_config.dor_port, 0u);
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_advance_at(fdc, observe(fdc).reset_due_tick);
                for (lib_u8 reset_drive = 0u;
                    reset_drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++reset_drive) {
                    failed |= !core_machine_fdc_command(fdc, port,
                        (const lib_u8[]){0x08u}, 1u);
                    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                        result[0] != (TEST_FDC_ST0_READY_CHANGE | reset_drive) ||
                        result[1] != 0u || fdc->connect.irq_source.asserted;
                }
                failed |= !core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));
                core_machine_port_write(port, fdc_config.control_port, VFDC_CCR_RATE_250);

                fixture.forced_read_result = CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
                failed |= !core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                (void)core_machine_port_read(port, fdc_config.data_port);
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[1] & 0x04u) == 0u;
                fixture.forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK;
                fixture.read_only = LIB_TRUE;
                failed |= !core_machine_fdc_command(fdc, port, write_sector, sizeof(write_sector));
                core_machine_port_write(port, fdc_config.data_port, 0x33u);
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[1] & 0x02u) == 0u;
                fixture.read_only = LIB_FALSE;
                fixture.present = LIB_FALSE;
                failed |= !core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    (result[1] & 0x04u) == 0u;

                /* The same MFM byte-service interval applies when the host
                   services 3F5h directly rather than through DMA2. */
                fixture.present = LIB_TRUE;
                fixture.read_count = 0u;
                core_machine_port_write(port, fdc_config.control_port, 0u);
                failed |= !core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));
                failed |= !core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                core_machine_fdc_advance_at(fdc, observe(fdc).elapsed_ticks + 1u);
                failed |= core_machine_port_read(port, fdc_config.data_port) != 0xa5u ||
                    fixture.read_count != 1u || (core_machine_port_read(port, 0x03f4u) & TEST_FDC_MSR_RQM) != 0u;
                failed |= core_machine_fdc_next_due_tick(fdc, &ndma_gate_tick) != LIB_STATUS_OK;
                failed |= ndma_gate_tick != observe(fdc).elapsed_ticks +
                    8u * 15u;
                core_machine_fdc_advance_at(fdc, ndma_gate_tick - 1u);
                failed |= (core_machine_port_read(port, fdc_config.status_port) & TEST_FDC_MSR_RQM) != 0u ||
                    fixture.read_count != 1u;
                core_machine_fdc_advance_at(fdc, ndma_gate_tick);
                failed |= (core_machine_port_read(port, fdc_config.status_port) &
                    TEST_FDC_MSR_PROCESS_READ) != TEST_FDC_MSR_PROCESS_READ;
                (void)core_machine_port_read(port, fdc_config.data_port);
                failed |= fixture.read_count != 2u;
                core_machine_port_write(port, fdc_config.dor_port, 0u);
                failed |= core_machine_fdc_next_due_tick(fdc, &ndma_gate_tick) != LIB_STATUS_INVALID_STATE;

                /* Scan is host-to-controller execution too.  This FM command
                   uses the 31-us byte gate, and DOR reset cancels it. */
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_fdc_advance_at(fdc, observe(fdc).reset_due_tick);
                for (lib_u8 reset_drive = 0u;
                    reset_drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++reset_drive) {
                    failed |= !core_machine_fdc_command(fdc, port,
                        (const lib_u8[]){0x08u}, 1u);
                    failed |= !core_machine_fdc_read_result(fdc, port, result, 2u) ||
                        result[0] != (TEST_FDC_ST0_READY_CHANGE | reset_drive) ||
                        result[1] != 0u || fdc->connect.irq_source.asserted;
                }
                failed |= !core_machine_fdc_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));
                core_machine_port_write(port, fdc_config.control_port, 0u);
                failed |= !core_machine_fdc_command(fdc, port, scan_equal, sizeof(scan_equal));
                core_machine_port_write(port, fdc_config.data_port, 0x5au);
                failed |= core_machine_fdc_next_due_tick(fdc, &ndma_gate_tick) != LIB_STATUS_OK;
                failed |= (core_machine_port_read(port, 0x03f4u) & TEST_FDC_MSR_RQM) != 0u ||
                    ndma_gate_tick != observe(fdc).elapsed_ticks +
                    8u * 31u;
                core_machine_port_write(port, fdc_config.dor_port, 0u);
                failed |= (core_machine_port_read(port, 0x03f4u) & TEST_FDC_MSR_CB) != 0u ||
                    core_machine_fdc_next_due_tick(fdc, &ndma_gate_tick) != LIB_STATUS_INVALID_STATE;

                /* An unqualified service-time conversion is still a complete
                   logical DRQ/DACK handshake: single-mode DMA may consume
                   successive bytes without a fabricated delay. */
                fdc->connect.config.clock_ticks_per_second = 0u;
                core_machine_port_write(port, fdc_config.control_port, VFDC_CCR_RATE_500);
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                failed |= !core_machine_fdc_command(fdc, port,
                    (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
                core_machine_fdc_write_dma2(port, 0x0600u, 1u);
                fallback_read_count = fixture.read_count;
                failed |= !core_machine_fdc_command(fdc, port, read_sector, sizeof(read_sector));
                test_dma_transfers(&machine->board->shared_dma_latch,
                    &machine->board->shared_dma_primary, &machine->board->shared_dma_secondary,
                    machine, &machine->executor_port, 1u);
                failed |= fixture.read_count != fallback_read_count + 1u ||
                    !observe(fdc).dma_byte_gate_pending ||
                    core_machine_dma_has_pending_request(&machine->board->shared_dma_primary,
                        &machine->board->shared_dma_secondary);
                core_machine_fdc_advance_at(fdc, observe(fdc).next_dma_byte_tick);
                failed |= !core_machine_dma_has_pending_request(&machine->board->shared_dma_primary,
                    &machine->board->shared_dma_secondary);
                test_dma_transfers(&machine->board->shared_dma_latch,
                    &machine->board->shared_dma_primary, &machine->board->shared_dma_secondary,
                    machine, &machine->executor_port, 1u);
                failed |= fixture.read_count != fallback_read_count + 2u || core_machine_port_read(port, 0x03f4u) != TEST_FDC_MSR_CB;
                test_fdc_advance(fdc);
                failed |= !core_machine_fdc_read_result(fdc, port, result, sizeof(result)) ||
                    result[0] != TEST_FDC_ST0_NORMAL || result[1] != 0u ||
                    result[2] != 0u;
            }
        }
    }
    if (!failed) failed |= core_machine_fdc_readiness_matrix(machine, &fixture);
    if (!failed) failed |= core_machine_fdc_result_identity(fdc, port, &fixture);
    if (!failed) failed |= core_machine_fdc_write_terminal(machine, &fixture);
    if (!failed) failed |= core_machine_fdc_terminal_id(machine, &fixture);
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
    puts("M5:T375:S24:FDC-NDMA-CADENCE:OK");
    puts("M5:T376:S3:8272A-DELETED-DATA:OK");
    puts("M5:T376:S4:8272A-SCAN:OK");
    puts("M5:T465:S2:FDC-reset:OK");
    puts("M5:T465:S3:FDC-8272-command:OK");
    puts("M5:T539:S12:FDC-readiness-matrix:OK");
    puts("M5:T539:S12:FDC-ST0-identity:OK");
    puts("M5:T539:S12:FDC-PCN-Track0-READY:OK");
    return 0;
}
