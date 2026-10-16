#include "lib/types/file.h"
#include "hdc_fixture.h"
#include "core/board-base/machine_board_interface.h"
#include "lib/types/types_interface.h"
#include "core/board-base/hdc.h"
#include "core/board-base/machine_board_state.h"
#include "core/board-base/media_interface.h"

typedef struct core_machine_hdc_fixture_media {
    lib_u8 sector[2][512];
    lib_u64 generation;
    lib_u32 query_count;
    lib_u32 read_count;
    lib_u32 write_count;
    lib_u8 present;
    lib_u8 read_only;
    core_machine_media_result forced_read_result;
    core_machine_media_result forced_write_result;
    lib_u32 format_count;
    core_machine_media_track formatted;
    lib_u8 ecc[2][4];
    lib_bool ecc_present[2];
} core_machine_hdc_fixture_media;

static core_machine_media_result core_machine_hdc_fixture_query(void *context,
    core_machine_media_info *out_info)
{
    core_machine_hdc_fixture_media *media = context;

    if (media == LIB_NULL || out_info == LIB_NULL) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->query_count;
    lib_memory_set(out_info, 0, sizeof(*out_info));
    out_info->generation = media->generation;
    out_info->present = media->present;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
        CORE_MACHINE_MEDIA_CAPABILITY_CHANGE_DETECTABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_FORMATTABLE;
    if (media->read_only) out_info->capabilities |= CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY;
    out_info->geometry.cylinders = 1u;
    out_info->geometry.heads = 1u;
    out_info->geometry.sectors_per_track = 2u;
    out_info->geometry.bytes_per_sector = 512u;
    out_info->geometry.logical_sector_count = 2u;
    return media->present ? CORE_MACHINE_MEDIA_RESULT_OK : CORE_MACHINE_MEDIA_RESULT_ABSENT;
}

static core_machine_media_result core_machine_hdc_fixture_read(void *context,
    lib_u64 offset, void *buffer, lib_u32 byte_count)
{
    core_machine_hdc_fixture_media *media = context;

    if (media == LIB_NULL || buffer == LIB_NULL || !media->present) {
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    }
    if (media->forced_read_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        return media->forced_read_result;
    }
    if ((offset != 0u && offset != 512u) || byte_count != sizeof(media->sector[0])) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->read_count;
    lib_memory_copy(buffer, media->sector[offset / 512u], sizeof(media->sector[0]));
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_hdc_fixture_write(void *context,
    lib_u64 offset, const void *buffer, lib_u32 byte_count)
{
    core_machine_hdc_fixture_media *media = context;

    if (media == LIB_NULL || buffer == LIB_NULL || !media->present) {
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    }
    if (media->read_only) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (media->forced_write_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        return media->forced_write_result;
    }
    if ((offset != 0u && offset != 512u) || byte_count != sizeof(media->sector[0])) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->write_count;
    lib_memory_copy(media->sector[offset / 512u], buffer, sizeof(media->sector[0]));
    media->ecc_present[offset / 512u] = LIB_FALSE;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_hdc_fixture_read_record(void *context,
    lib_u64 sector, core_machine_media_record *record)
{
    core_machine_hdc_fixture_media *media = context;
    core_machine_media_result result;

    if (sector >= 2u) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    result = core_machine_hdc_fixture_read(context, sector * 512u, record->data, 512u);
    if (result == CORE_MACHINE_MEDIA_RESULT_OK) {
        lib_memory_copy(record->ecc, media->ecc[sector], sizeof(record->ecc));
        record->ecc_present = media->ecc_present[sector];
    }
    return result;
}

static core_machine_media_result core_machine_hdc_fixture_write_record(void *context,
    lib_u64 sector, const core_machine_media_record *record)
{
    core_machine_hdc_fixture_media *media = context;
    core_machine_media_result result;

    if (sector >= 2u) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    result = core_machine_hdc_fixture_write(context, sector * 512u, record->data, 512u);
    if (result == CORE_MACHINE_MEDIA_RESULT_OK) {
        lib_memory_copy(media->ecc[sector], record->ecc, sizeof(record->ecc));
        media->ecc_present[sector] = record->ecc_present;
    }
    return result;
}

static core_machine_media_result core_machine_hdc_fixture_format(void *context,
    const core_machine_media_track *track)
{
    core_machine_hdc_fixture_media *media = context;
    ++media->format_count;
    media->formatted = *track;
    if (track->fill_payload) lib_memory_set(media->sector, track->fill, sizeof(media->sector));
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider core_machine_hdc_fixture_provider = {
    .query = core_machine_hdc_fixture_query,
    .read_bytes = core_machine_hdc_fixture_read,
    .write_bytes = core_machine_hdc_fixture_write,
    .format_track = core_machine_hdc_fixture_format,
    .read_record = core_machine_hdc_fixture_read_record,
    .write_record = core_machine_hdc_fixture_write_record,
};

static lib_i32 core_machine_hdc_write(x86_core_machine *machine, lib_u16 port,
    lib_u32 value)
{
    return x86_core_machine_bus_write(machine, port, value) == LIB_STATUS_OK;
}

static lib_i32 core_machine_hdc_read(x86_core_machine *machine, lib_u16 port,
    lib_u32 *out_value)
{
    return x86_core_machine_bus_read(machine, port, out_value) == LIB_STATUS_OK;
}

static lib_i32 core_machine_hdc_command(x86_core_machine *machine, core_machine_board_state *board,
    const core_machine_hdc_task_file_config *config, lib_u8 command)
{
    if (!core_machine_hdc_write(machine, config->status_command_port, command)) return 0;
    hdc_service(board->hdc);
    return 1;
}

static lib_i32 core_machine_hdc_program_chs(x86_core_machine *machine,
    const core_machine_hdc_task_file_config *config)
{
    return core_machine_hdc_write(machine, config->sector_count_port, 1u) &&
        core_machine_hdc_write(machine, config->sector_number_port, 1u) &&
        core_machine_hdc_write(machine, config->cylinder_low_port, 0u) &&
        core_machine_hdc_write(machine, config->cylinder_high_port, 0u) &&
        core_machine_hdc_write(machine, config->drive_head_port, 0u);
}

static lib_i32 core_machine_hdc_drain(x86_core_machine *machine, core_machine_board_state *board,
    const core_machine_hdc_task_file_config *config, lib_u16 *first_word)
{
    lib_u32 word;

    for (lib_u32 index = 0u; index < 256u; ++index) {
        if (!core_machine_hdc_read(machine, config->data_port, &word)) return 0;
        if (index == 0u && first_word != LIB_NULL) *first_word = (lib_u16)word;
    }
    hdc_service(board->hdc);
    return 1;
}

static lib_i32 core_machine_hdc_drain_long(x86_core_machine *machine,
    core_machine_board_state *board, const core_machine_hdc_task_file_config *config,
    lib_u16 *first_word)
{
    lib_u32 byte;
    if (!core_machine_hdc_drain(machine, board, config, first_word)) return 0;
    if (hdc_observe(board->hdc).phase != X86_HDC_PHASE_DATA_READ ||
        hdc_observe(board->hdc).data_index != 512u) return 0;
    for (lib_u32 index = 0u; index < 4u; ++index)
        if (!core_machine_hdc_read(machine, config->data_port, &byte) || byte > 0xffu)
            return 0;
    hdc_service(board->hdc);
    return hdc_observe(board->hdc).phase == X86_HDC_PHASE_IDLE;
}

static lib_i32 core_machine_hdc_fill(x86_core_machine *machine, core_machine_board_state *board,
    const core_machine_hdc_task_file_config *config, lib_u16 first_word)
{
    for (lib_u32 index = 0u; index < 256u; ++index) {
        if (!core_machine_hdc_write(machine, config->data_port,
                index == 0u ? first_word : 0u)) return 0;
    }
    hdc_service(board->hdc);
    return 1;
}

static lib_i32 core_machine_hdc_test_ibm_wd1003(void)
{
    const core_machine_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_8086,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_hdc_task_file_config hdc_config = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .lba28_supported = LIB_FALSE, .clock_ticks_per_second = 8000000u
    };
    const core_machine_hdc_config hdc_plan = {
        .protocol = CORE_MACHINE_HDC_PROTOCOL_IBM_WD1003_ST506,
        .irq = 14u, .bus.task_file = hdc_config
    };
    core_machine_hdc_fixture_media media = {
        .generation = 1u, .present = LIB_TRUE,
        .forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK,
        .forced_write_result = CORE_MACHINE_MEDIA_RESULT_OK
    };
    core_machine_media_registry *registry = LIB_NULL;
    core_machine_hdc_topology topology = {0};
    x86_core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_u32 status = 0u;
    lib_u16 word = 0u;
    lib_i32 failed = 0;

    media.sector[0][0] = 0x78u;
    media.sector[0][1] = 0x56u;
    if (core_machine_media_registry_create(&registry) != LIB_STATUS_OK ||
        core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_media_registry_bind(registry, 1u, &media,
            &core_machine_hdc_fixture_provider) != LIB_STATUS_OK ||
        core_machine_media_registry_freeze(registry) != LIB_STATUS_OK) {
        failed = 1;
    } else {
        topology.media_registry = registry;
        topology.media_id = 1u;
            topology.config = hdc_plan;
        if (core_machine_configure_hdc(board, &topology) != LIB_STATUS_OK ||
            x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            hdc_observe(board->hdc).error != X86_HDC_ERROR_DIAGNOSTIC_OK ||
            !core_machine_hdc_program_chs(machine, &hdc_config) ||
            !core_machine_hdc_write(machine, hdc_config.drive_head_port, 0xa0u) ||
            !core_machine_hdc_command(machine, board, &hdc_config, 0x22u) ||
            !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
            status != (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC | X86_HDC_STATUS_DRQ) ||
            hdc_observe(board->hdc).error != 0u ||
            !core_machine_hdc_drain_long(machine, board, &hdc_config, &word) || word != 0x5678u ||
            !core_machine_hdc_write(machine, hdc_config.sector_count_port, 1u) ||
            !core_machine_hdc_command(machine, board, &hdc_config, 0x20u) ||
            !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
            status != (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC |
                X86_HDC_STATUS_DRQ) ||
            !core_machine_hdc_drain(machine, board, &hdc_config, &word) || word != 0x5678u ||
            !core_machine_hdc_write(machine,
                hdc_config.alternate_status_device_control_port, 0x08u) ||
            hdc_observe(board->hdc).fixed_disk_register != 0x08u ||
            hdc_observe(board->hdc).drive_head != 0xa0u ||
            x86_core_machine_bus_read(machine, hdc_config.alternate_status_device_control_port,
                &status) != LIB_STATUS_UNSUPPORTED ||
            !core_machine_hdc_program_chs(machine, &hdc_config) ||
            !core_machine_hdc_command(machine, board, &hdc_config, 0x91u) ||
            !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
            status != (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC) ||
            !core_machine_hdc_command(machine, board, &hdc_config, 0x10u) ||
            hdc_observe(board->hdc).step_rate_selector != 0u ||
            hdc_observe(board->hdc).step_rate_ticks != 280u ||
            hdc_observe(board->hdc).step_pulse_limit != 1023u ||
            !core_machine_hdc_command(machine, board, &hdc_config, 0x7fu) ||
            hdc_observe(board->hdc).step_rate_selector != 15u ||
            hdc_observe(board->hdc).step_rate_ticks != 60000u ||
            hdc_observe(board->hdc).step_pulse_limit != 0u ||
            !core_machine_hdc_command(machine, board, &hdc_config, 0x90u) ||
            hdc_observe(board->hdc).step_rate_selector != 15u ||
            hdc_observe(board->hdc).step_rate_ticks != 60000u ||
            hdc_observe(board->hdc).step_pulse_limit != 1023u ||
            !core_machine_hdc_command(machine, board, &hdc_config, 0xecu) ||
            !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
            status != (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_ERR) ||
            hdc_observe(board->hdc).error != X86_HDC_ERROR_ABORT) {
            failed = 1;
        }
    }
    if (!failed) {
        if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
            !core_machine_hdc_write(machine, hdc_config.sector_count_port, 2u) ||
            !core_machine_hdc_command(machine, board, &hdc_config, 0x91u) ||
            !core_machine_hdc_command(machine, board, &hdc_config, 0x50u) ||
            core_machine_hdc_irq_pending(board->hdc)) failed = 1;
        for (lib_u32 index = 0u; index < 255u; ++index)
            if (!core_machine_hdc_write(machine, hdc_config.data_port,
                    index == 0u ? 0x0200u : index == 1u ? 0x0180u : 0u)) failed = 1;
        if (media.format_count != 0u ||
            !core_machine_hdc_write(machine, hdc_config.data_port, 0u)) failed = 1;
        hdc_service(board->hdc);
        if (media.format_count != 1u || media.formatted.ids[0] != 1u ||
            media.formatted.ids[1] != 0u || !media.formatted.bad[1] ||
            !media.formatted.fill_payload || media.sector[0][0] != 0u ||
            !core_machine_hdc_irq_pending(board->hdc) ||
            !core_machine_pic_irq_source_is_asserted(board->hdc->connect.irq_source)) failed = 1;
    }
    if (!failed) {
        core_machine_hdc_write(machine, hdc_config.sector_number_port, 2u);
        core_machine_hdc_write(machine, hdc_config.sector_count_port, 1u);
        core_machine_hdc_command(machine, board, &hdc_config, 0x32u);
        for (lib_u32 index = 0u; index < 256u; ++index)
            core_machine_hdc_write(machine, hdc_config.data_port, 0x4a4au);
        if (media.ecc_present[1] || hdc_observe(board->hdc).data_index != 512u)
            failed = 1;
        for (lib_u32 index = 0u; index < 4u; ++index)
            core_machine_hdc_write(machine, hdc_config.data_port, 0x21u + index);
        hdc_service(board->hdc);
        if (!media.ecc_present[1] || media.ecc[1][3] != 0x24u ||
            !core_machine_hdc_irq_pending(board->hdc)) failed = 1;
        core_machine_hdc_write(machine, hdc_config.sector_count_port, 1u);
        core_machine_hdc_command(machine, board, &hdc_config, 0x22u);
        core_machine_hdc_read(machine, hdc_config.status_command_port, &status);
        for (lib_u32 index = 0u; index < 256u; ++index) {
            core_machine_hdc_read(machine, hdc_config.data_port, &status);
            if (status != 0x4a4au) failed = 1;
        }
        if (hdc_observe(board->hdc).data_index != 512u) failed = 1;
        for (lib_u32 index = 0u; index < 4u; ++index) {
            core_machine_hdc_read(machine, hdc_config.data_port, &status);
            if (status != 0x21u + index) failed = 1;
        }
        hdc_service(board->hdc);
        if (hdc_observe(board->hdc).phase != X86_HDC_PHASE_IDLE) failed = 1;
    }
    x86_core_machine_destroy(machine);
    core_machine_media_registry_destroy(registry);
    return failed;
}

lib_i32 main(void)
{
    const core_machine_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_8086,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_hdc_task_file_config hdc_config = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .lba28_supported = LIB_TRUE
    };
    const core_machine_hdc_config hdc_plan = {
        .protocol = CORE_MACHINE_HDC_PROTOCOL_ATA_PIO,
        .irq = 14u, .service = {7u, 3u}, .bus.task_file = hdc_config
    };
    core_machine_hdc_fixture_media media = {
        .generation = 1u, .present = LIB_TRUE,
        .forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK,
        .forced_write_result = CORE_MACHINE_MEDIA_RESULT_OK
    };
    core_machine_media_registry *registry = LIB_NULL;
    core_machine_hdc_topology topology = {0};
    x86_core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_hdc *hdc;
    lib_u32 status = 0u;
    lib_u32 error = 0u;
    lib_u16 word = 0u;
    lib_u32 queries_before;
    lib_u32 reads_before;
    lib_i32 failed = 0;

    media.sector[0][0] = 0x34u;
    media.sector[0][1] = 0x12u;
    if (core_machine_media_registry_create(&registry) != LIB_STATUS_OK ||
        core_machine_create(&config, &machine, &board) != LIB_STATUS_OK) failed |= 0x01;
    if (!failed) {
        hdc = board->hdc;
        if (hdc == LIB_NULL ||
            core_machine_media_registry_bind(registry, 1u, &media,
                &core_machine_hdc_fixture_provider) != LIB_STATUS_OK ||
            core_machine_media_registry_freeze(registry) != LIB_STATUS_OK ||
            core_machine_media_registry_bind(registry, 2u, &media,
                &core_machine_hdc_fixture_provider) != LIB_STATUS_INVALID_STATE) {
            failed |= 0x02;
        } else {
            topology.media_registry = registry;
            topology.media_id = 1u;
            topology.config = (core_machine_hdc_config) {0};
            if (core_machine_configure_hdc(board, &topology) !=
                LIB_STATUS_INVALID_ARGUMENT) {
                failed |= 0x04;
            }
            topology.config = hdc_plan;
            if (!failed && (core_machine_configure_hdc(board, &topology) != LIB_STATUS_OK ||
                x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                x86_core_machine_reset(machine) != LIB_STATUS_OK)) {
                failed |= 0x04;
            } else {
                if (hdc_observe(hdc).error != 0x01u || hdc_observe(hdc).sector_count != 1u ||
                    hdc_observe(hdc).sector_number != 1u ||
                    !core_machine_hdc_write(machine,
                        hdc_config.alternate_status_device_control_port,
                        X86_HDC_DEVICE_CONTROL_NIEN) ||
                    !core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, board, &hdc_config, 0x20u) ||
                    !core_machine_hdc_read(machine,
                        hdc_config.alternate_status_device_control_port, &status) ||
                    status != (X86_HDC_STATUS_DRDY |
                        X86_HDC_STATUS_DSC | X86_HDC_STATUS_DRQ) ||
                    core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_write(machine,
                        hdc_config.alternate_status_device_control_port, 0u) ||
                    !core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_drain(machine, board, &hdc_config, &word) ||
                    word != 0x1234u ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    !core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, board, &hdc_config, 0x20u) ||
                    !core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port,
                        &status) ||
                    !core_machine_hdc_drain(machine, board, &hdc_config, &word) ||
                    word != 0x1234u) failed |= 0x400;

                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, board, &hdc_config, 0x20u) ||
                    !core_machine_hdc_drain(machine, board, &hdc_config, &word) || word != 0x1234u ||
                    media.read_count != 3u) failed |= 0x08;

                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, board, &hdc_config, 0x30u) ||
                    !core_machine_hdc_fill(machine, board, &hdc_config, 0xa55au) ||
                    media.write_count != 1u || media.sector[0][0] != 0x5au ||
                    media.sector[0][1] != 0xa5u) failed |= 0x10;

                queries_before = media.query_count;
                ++media.generation;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, board, &hdc_config, 0x20u) ||
                    !core_machine_hdc_drain(machine, board, &hdc_config, &word) || word != 0xa55au ||
                    media.query_count <= queries_before) failed |= 0x20;

                media.forced_read_result = CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, board, &hdc_config, 0x20u) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    !core_machine_hdc_read(machine, hdc_config.error_features_port, &error) ||
                    status != (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_ERR) ||
                    error != X86_HDC_ERROR_ID_NOT_FOUND) failed |= 0x40;
                media.forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK;

                reads_before = media.read_count;
                media.forced_read_result = CORE_MACHINE_MEDIA_RESULT_PERMANENT;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, board, &hdc_config, 0x30u) ||
                    hdc_observe(hdc).phase != X86_HDC_PHASE_DATA_WRITE ||
                    !core_machine_hdc_fill(machine, board, &hdc_config, 0xa55au) ||
                    media.read_count != reads_before || media.write_count != 2u ||
                    media.sector[0][0] != 0x5au || media.sector[0][1] != 0xa5u) {
                    failed |= 0x10000;
                }
                media.forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK;

                media.read_only = LIB_TRUE;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, board, &hdc_config, 0x30u) ||
                    !core_machine_hdc_fill(machine, board, &hdc_config, 0xbeefu) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    !core_machine_hdc_read(machine, hdc_config.error_features_port, &error) ||
                    status != (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_ERR) ||
                    error != X86_HDC_ERROR_ABORT) failed |= 0x80;
                media.read_only = LIB_FALSE;
                media.present = LIB_FALSE;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, board, &hdc_config, 0x20u) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    status != (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_ERR)) {
                    failed |= 0x100;
                }
            }
        }
    }
    x86_core_machine_destroy(machine);
    core_machine_media_registry_destroy(registry);
    failed |= core_machine_hdc_test_ibm_wd1003();
    if (failed) {
        lib_c_fprintf(lib_c_stderr, "ATA-NIEN:PORT:FAIL bits=%x status=%02x error=%02x word=%04x\n",
            failed, status, error, word);
        return 1;
    }
    lib_c_printf("%s\n", "ATA-NIEN:PORT:OK");
    lib_c_printf("%s\n", "CORE-HDC-MEDIA:OK");
    lib_c_printf("%s\n", "IBM-WD1003:OK");
    return 0;
}
