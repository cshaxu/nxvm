#include "type.h"

#include "core/machine/hdc.h"
#include "core/machine/machine.h"
#include "core/machine/media_interface.h"

typedef struct core_machine_hdc_fixture_media {
    type_unsigned_8 sector[2][512];
    type_unsigned_64 generation;
    type_unsigned_32 query_count;
    type_unsigned_32 read_count;
    type_unsigned_32 write_count;
    type_bool present;
    type_bool read_only;
    core_machine_media_result forced_read_result;
    core_machine_media_result forced_write_result;
} core_machine_hdc_fixture_media;

static core_machine_media_result core_machine_hdc_fixture_query(C_VOID *context,
    core_machine_media_info *out_info)
{
    core_machine_hdc_fixture_media *media = context;

    if (media == STD_NULL || out_info == STD_NULL) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->query_count;
    STD_MEMSET(out_info, 0, sizeof(*out_info));
    out_info->generation = media->generation;
    out_info->present = media->present;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
        CORE_MACHINE_MEDIA_CAPABILITY_CHANGE_DETECTABLE;
    if (media->read_only) out_info->capabilities |= CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY;
    out_info->geometry.cylinders = 1u;
    out_info->geometry.heads = 1u;
    out_info->geometry.sectors_per_track = 2u;
    out_info->geometry.bytes_per_sector = 512u;
    out_info->geometry.logical_sector_count = 2u;
    return media->present ? CORE_MACHINE_MEDIA_RESULT_OK : CORE_MACHINE_MEDIA_RESULT_ABSENT;
}

static core_machine_media_result core_machine_hdc_fixture_read(C_VOID *context,
    type_unsigned_64 offset, C_VOID *buffer, type_unsigned_32 byte_count)
{
    core_machine_hdc_fixture_media *media = context;

    if (media == STD_NULL || buffer == STD_NULL || !media->present) {
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    }
    if (media->forced_read_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        return media->forced_read_result;
    }
    if ((offset != 0u && offset != 512u) || byte_count != sizeof(media->sector[0])) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->read_count;
    STD_MEMCPY(buffer, media->sector[offset / 512u], sizeof(media->sector[0]));
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_hdc_fixture_write(C_VOID *context,
    type_unsigned_64 offset, const C_VOID *buffer, type_unsigned_32 byte_count)
{
    core_machine_hdc_fixture_media *media = context;

    if (media == STD_NULL || buffer == STD_NULL || !media->present) {
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
    STD_MEMCPY(media->sector[offset / 512u], buffer, sizeof(media->sector[0]));
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider core_machine_hdc_fixture_provider = {
    core_machine_hdc_fixture_query,
    core_machine_hdc_fixture_read,
    core_machine_hdc_fixture_write,
    STD_NULL,
    STD_NULL,
    STD_NULL,
    STD_NULL
};

static C_INT core_machine_hdc_write(core_machine *machine, type_unsigned_16 port,
    type_unsigned_32 value)
{
    return core_machine_bus_write(machine, port, value) == TYPE_STATUS_OK;
}

static C_INT core_machine_hdc_read(core_machine *machine, type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    return core_machine_bus_read(machine, port, out_value) == TYPE_STATUS_OK;
}

static C_INT core_machine_hdc_command(core_machine *machine,
    const core_machine_hdc_task_file_config *config, type_unsigned_8 command)
{
    if (!core_machine_hdc_write(machine, config->status_command_port, command)) return 0;
    core_machine_hdc_advance(&machine->hdc);
    return 1;
}

static C_INT core_machine_hdc_program_chs(core_machine *machine,
    const core_machine_hdc_task_file_config *config)
{
    return core_machine_hdc_write(machine, config->sector_count_port, 1u) &&
        core_machine_hdc_write(machine, config->sector_number_port, 1u) &&
        core_machine_hdc_write(machine, config->cylinder_low_port, 0u) &&
        core_machine_hdc_write(machine, config->cylinder_high_port, 0u) &&
        core_machine_hdc_write(machine, config->drive_head_port, 0u);
}

static C_INT core_machine_hdc_drain(core_machine *machine,
    const core_machine_hdc_task_file_config *config, type_unsigned_16 *first_word)
{
    type_unsigned_32 word;

    for (type_unsigned_32 index = 0u; index < 256u; ++index) {
        if (!core_machine_hdc_read(machine, config->data_port, &word)) return 0;
        if (index == 0u && first_word != STD_NULL) *first_word = (type_unsigned_16)word;
    }
    core_machine_hdc_advance(&machine->hdc);
    return 1;
}

static C_INT core_machine_hdc_fill(core_machine *machine,
    const core_machine_hdc_task_file_config *config, type_unsigned_16 first_word)
{
    for (type_unsigned_32 index = 0u; index < 256u; ++index) {
        if (!core_machine_hdc_write(machine, config->data_port,
                index == 0u ? first_word : 0u)) return 0;
    }
    core_machine_hdc_advance(&machine->hdc);
    return 1;
}

static C_INT core_machine_hdc_test_ibm_wd1003(C_VOID)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_hdc_task_file_config hdc_config = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .lba28_supported = TYPE_FALSE, .clock_ticks_per_second = 8000000u
    };
    const core_machine_hdc_config hdc_plan = {
        .protocol = CORE_MACHINE_HDC_PROTOCOL_IBM_WD1003_ST506,
        .irq = 14u, .bus.task_file = hdc_config
    };
    core_machine_hdc_fixture_media media = {
        .generation = 1u, .present = TYPE_TRUE,
        .forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK,
        .forced_write_result = CORE_MACHINE_MEDIA_RESULT_OK
    };
    core_machine_media_registry *registry = STD_NULL;
    core_machine_hdc_topology topology = {0};
    core_machine *machine = STD_NULL;
    type_unsigned_32 status = 0u;
    type_unsigned_16 word = 0u;
    C_INT failed = 0;

    media.sector[0][0] = 0x78u;
    media.sector[0][1] = 0x56u;
    if (core_machine_media_registry_create(&registry) != TYPE_STATUS_OK ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(registry, 1u, &media,
            &core_machine_hdc_fixture_provider) != TYPE_STATUS_OK ||
        core_machine_media_registry_freeze(registry) != TYPE_STATUS_OK) {
        failed = 1;
    } else {
        topology.media_registry = registry;
        topology.media_id = 1u;
            topology.config = hdc_plan;
        if (core_machine_configure_hdc(machine, &topology) != TYPE_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
            core_machine_reset(machine) != TYPE_STATUS_OK ||
            machine->hdc.data.error != CORE_MACHINE_HDC_ERROR_DIAGNOSTIC_OK ||
            !core_machine_hdc_program_chs(machine, &hdc_config) ||
            !core_machine_hdc_write(machine, hdc_config.drive_head_port, 0xa0u) ||
            !core_machine_hdc_command(machine, &hdc_config, 0x22u) ||
            !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
            status != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC |
                CORE_MACHINE_HDC_STATUS_DRQ) ||
            !core_machine_hdc_drain(machine, &hdc_config, &word) || word != 0x5678u ||
            !core_machine_hdc_write(machine,
                hdc_config.alternate_status_device_control_port, 0x08u) ||
            machine->hdc.data.fixed_disk_register != 0x08u ||
            machine->hdc.data.drive_head != 0xa0u ||
            core_machine_bus_read(machine, hdc_config.alternate_status_device_control_port,
                &status) != TYPE_STATUS_UNSUPPORTED ||
            !core_machine_hdc_program_chs(machine, &hdc_config) ||
            !core_machine_hdc_command(machine, &hdc_config, 0x91u) ||
            !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
            status != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC) ||
            !core_machine_hdc_command(machine, &hdc_config, 0x10u) ||
            machine->hdc.data.step_rate_selector != 0u ||
            machine->hdc.data.step_rate_ticks != 280u ||
            machine->hdc.data.step_pulse_limit != 1023u ||
            !core_machine_hdc_command(machine, &hdc_config, 0x7fu) ||
            machine->hdc.data.step_rate_selector != 15u ||
            machine->hdc.data.step_rate_ticks != 60000u ||
            machine->hdc.data.step_pulse_limit != 0u ||
            !core_machine_hdc_command(machine, &hdc_config, 0x90u) ||
            machine->hdc.data.step_rate_selector != 15u ||
            machine->hdc.data.step_rate_ticks != 60000u ||
            machine->hdc.data.step_pulse_limit != 1023u ||
            !core_machine_hdc_command(machine, &hdc_config, 0xecu) ||
            !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
            status != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_ERR) ||
            machine->hdc.data.error != CORE_MACHINE_HDC_ERROR_ABORT) {
            failed = 1;
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(registry);
    return failed;
}

C_INT main(C_VOID)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_hdc_task_file_config hdc_config = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .lba28_supported = TYPE_TRUE
    };
    const core_machine_hdc_config hdc_plan = {
        .protocol = CORE_MACHINE_HDC_PROTOCOL_ATA_PIO,
        .irq = 14u, .bus.task_file = hdc_config
    };
    core_machine_hdc_fixture_media media = {
        .generation = 1u, .present = TYPE_TRUE,
        .forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK,
        .forced_write_result = CORE_MACHINE_MEDIA_RESULT_OK
    };
    core_machine_media_registry *registry = STD_NULL;
    core_machine_hdc_topology topology = {0};
    core_machine *machine = STD_NULL;
    core_machine_hdc *hdc;
    type_unsigned_32 status = 0u;
    type_unsigned_32 error = 0u;
    type_unsigned_16 word = 0u;
    type_unsigned_32 queries_before;
    type_unsigned_32 reads_before;
    C_INT failed = 0;

    media.sector[0][0] = 0x34u;
    media.sector[0][1] = 0x12u;
    if (core_machine_media_registry_create(&registry) != TYPE_STATUS_OK ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK) failed |= 0x01;
    if (!failed) {
        hdc = &machine->hdc;
        if (hdc == STD_NULL ||
            core_machine_media_registry_bind(registry, 1u, &media,
                &core_machine_hdc_fixture_provider) != TYPE_STATUS_OK ||
            core_machine_media_registry_freeze(registry) != TYPE_STATUS_OK ||
            core_machine_media_registry_bind(registry, 2u, &media,
                &core_machine_hdc_fixture_provider) != TYPE_STATUS_INVALID_STATE) {
            failed |= 0x02;
        } else {
            topology.media_registry = registry;
            topology.media_id = 1u;
            topology.config = (core_machine_hdc_config) {0};
            if (core_machine_configure_hdc(machine, &topology) !=
                TYPE_STATUS_INVALID_ARGUMENT) {
                failed |= 0x04;
            }
            topology.config = hdc_plan;
            if (!failed && (core_machine_configure_hdc(machine, &topology) != TYPE_STATUS_OK ||
                core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
                core_machine_reset(machine) != TYPE_STATUS_OK)) {
                failed |= 0x04;
            } else {
                if (hdc->data.error != 0x01u || hdc->data.sector_count != 1u ||
                    hdc->data.sector_number != 1u ||
                    !core_machine_hdc_write(machine,
                        hdc_config.alternate_status_device_control_port,
                        CORE_MACHINE_HDC_DEVICE_CONTROL_NIEN) ||
                    !core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, &hdc_config, 0x20u) ||
                    !core_machine_hdc_read(machine,
                        hdc_config.alternate_status_device_control_port, &status) ||
                    status != (CORE_MACHINE_HDC_STATUS_DRDY |
                        CORE_MACHINE_HDC_STATUS_DSC | CORE_MACHINE_HDC_STATUS_DRQ) ||
                    core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_write(machine,
                        hdc_config.alternate_status_device_control_port, 0u) ||
                    !core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_drain(machine, &hdc_config, &word) ||
                    word != 0x1234u ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    !core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, &hdc_config, 0x20u) ||
                    !core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port,
                        &status) ||
                    !core_machine_hdc_drain(machine, &hdc_config, &word) ||
                    word != 0x1234u) failed |= 0x400;

                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, &hdc_config, 0x20u) ||
                    !core_machine_hdc_drain(machine, &hdc_config, &word) || word != 0x1234u ||
                    media.read_count != 3u) failed |= 0x08;

                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, &hdc_config, 0x30u) ||
                    !core_machine_hdc_fill(machine, &hdc_config, 0xa55au) ||
                    media.write_count != 1u || media.sector[0][0] != 0x5au ||
                    media.sector[0][1] != 0xa5u) failed |= 0x10;

                queries_before = media.query_count;
                ++media.generation;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, &hdc_config, 0x20u) ||
                    !core_machine_hdc_drain(machine, &hdc_config, &word) || word != 0xa55au ||
                    media.query_count <= queries_before) failed |= 0x20;

                media.forced_read_result = CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, &hdc_config, 0x20u) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    !core_machine_hdc_read(machine, hdc_config.error_features_port, &error) ||
                    status != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_ERR) ||
                    error != CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND) failed |= 0x40;
                media.forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK;

                reads_before = media.read_count;
                media.forced_read_result = CORE_MACHINE_MEDIA_RESULT_PERMANENT;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, &hdc_config, 0x30u) ||
                    hdc->data.phase != CORE_MACHINE_HDC_PHASE_DATA_WRITE ||
                    !core_machine_hdc_fill(machine, &hdc_config, 0xa55au) ||
                    media.read_count != reads_before || media.write_count != 2u ||
                    media.sector[0][0] != 0x5au || media.sector[0][1] != 0xa5u) {
                    failed |= 0x10000;
                }
                media.forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK;

                media.read_only = TYPE_TRUE;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, &hdc_config, 0x30u) ||
                    !core_machine_hdc_fill(machine, &hdc_config, 0xbeefu) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    !core_machine_hdc_read(machine, hdc_config.error_features_port, &error) ||
                    status != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_ERR) ||
                    error != CORE_MACHINE_HDC_ERROR_ABORT) failed |= 0x80;
                media.read_only = TYPE_FALSE;

                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_write(machine, hdc_config.status_command_port, 0x20u) ||
                    hdc->data.phase != CORE_MACHINE_HDC_PHASE_PENDING_COMMAND ||
                    hdc->data.status != CORE_MACHINE_HDC_STATUS_BSY ||
                    core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_write(machine, hdc_config.sector_number_port, 0u) ||
                    hdc->data.sector_number != 1u) {
                    failed |= 0x200;
                }
                core_machine_hdc_advance(hdc);
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_DATA_READ ||
                    hdc->data.sector_number != 1u ||
                    hdc->data.status != (CORE_MACHINE_HDC_STATUS_DRDY |
                        CORE_MACHINE_HDC_STATUS_DSC | CORE_MACHINE_HDC_STATUS_DRQ) ||
                    !core_machine_hdc_write(machine, hdc_config.status_command_port, 0x30u) ||
                    hdc->data.phase != CORE_MACHINE_HDC_PHASE_DATA_READ ||
                    hdc->data.status != (CORE_MACHINE_HDC_STATUS_DRDY |
                        CORE_MACHINE_HDC_STATUS_DSC | CORE_MACHINE_HDC_STATUS_DRQ) ||
                    !core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status);
                for (type_unsigned_32 index = 0u; index < 256u; ++index) {
                    failed |= !core_machine_hdc_read(machine, hdc_config.data_port, &status);
                    if (index == 0u) failed |= status != 0xa55au;
                }
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_PENDING_READ_SECTOR ||
                    hdc->data.status != CORE_MACHINE_HDC_STATUS_BSY ||
                    core_machine_hdc_irq_pending(hdc);
                core_machine_hdc_advance(hdc);
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_IDLE ||
                    hdc->data.status != (CORE_MACHINE_HDC_STATUS_DRDY |
                        CORE_MACHINE_HDC_STATUS_DSC) || !core_machine_hdc_irq_pending(hdc);

                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_write(machine, hdc_config.status_command_port, 0x30u)) {
                    failed |= 0x800;
                }
                core_machine_hdc_advance(hdc);
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_DATA_WRITE ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status);
                for (type_unsigned_32 index = 0u; index < 256u; ++index) {
                    failed |= !core_machine_hdc_write(machine, hdc_config.data_port,
                        index == 0u ? 0x5aa5u : 0u);
                }
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_PENDING_WRITE_SECTOR ||
                    hdc->data.status != CORE_MACHINE_HDC_STATUS_BSY ||
                    core_machine_hdc_irq_pending(hdc);
                core_machine_hdc_advance(hdc);
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_IDLE ||
                    media.sector[0][0] != 0xa5u || media.sector[0][1] != 0x5au ||
                    !core_machine_hdc_irq_pending(hdc);

                media.sector[1][0] = 0x78u;
                media.sector[1][1] = 0x56u;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_write(machine, hdc_config.sector_count_port, 2u) ||
                    !core_machine_hdc_command(machine, &hdc_config, 0x20u) ||
                    !core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    core_machine_hdc_irq_pending(hdc)) failed |= 0x4000;
                for (type_unsigned_32 index = 0u; index < 256u; ++index) {
                    failed |= !core_machine_hdc_read(machine, hdc_config.data_port, &status);
                    if (index == 0u) failed |= status != 0x5aa5u;
                }
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_PENDING_READ_SECTOR ||
                    hdc->data.status != CORE_MACHINE_HDC_STATUS_BSY || core_machine_hdc_irq_pending(hdc);
                core_machine_hdc_advance(hdc);
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_DATA_READ ||
                    hdc->data.sector_number != 2u || hdc->data.sector_count != 1u ||
                    !core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_read(machine, hdc_config.alternate_status_device_control_port, &status) ||
                    !core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    core_machine_hdc_irq_pending(hdc);
                for (type_unsigned_32 index = 0u; index < 256u; ++index) {
                    failed |= !core_machine_hdc_read(machine, hdc_config.data_port, &status);
                    if (index == 0u) failed |= status != 0x5678u;
                }
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_PENDING_READ_SECTOR ||
                    hdc->data.status != CORE_MACHINE_HDC_STATUS_BSY || core_machine_hdc_irq_pending(hdc);
                core_machine_hdc_advance(hdc);
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_IDLE ||
                    hdc->data.sector_count != 0u || !core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    core_machine_hdc_irq_pending(hdc);

                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_write(machine, hdc_config.sector_count_port, 2u) ||
                    !core_machine_hdc_command(machine, &hdc_config, 0x30u) ||
                    !core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    core_machine_hdc_irq_pending(hdc)) failed |= 0x8000;
                for (type_unsigned_32 index = 0u; index < 256u; ++index) {
                    failed |= !core_machine_hdc_write(machine, hdc_config.data_port,
                        index == 0u ? 0x2211u : 0u);
                }
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_PENDING_WRITE_SECTOR ||
                    hdc->data.status != CORE_MACHINE_HDC_STATUS_BSY ||
                    core_machine_hdc_irq_pending(hdc);
                core_machine_hdc_advance(hdc);
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_DATA_WRITE ||
                    hdc->data.sector_number != 2u || hdc->data.sector_count != 1u ||
                    media.write_count != 4u || media.sector[0][0] != 0x11u ||
                    media.sector[0][1] != 0x22u || !core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_read(machine,
                        hdc_config.alternate_status_device_control_port, &status) ||
                    !core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    core_machine_hdc_irq_pending(hdc);
                for (type_unsigned_32 index = 0u; index < 256u; ++index) {
                    failed |= !core_machine_hdc_write(machine, hdc_config.data_port,
                        index == 0u ? 0x4433u : 0u);
                }
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_PENDING_WRITE_SECTOR ||
                    hdc->data.status != CORE_MACHINE_HDC_STATUS_BSY ||
                    core_machine_hdc_irq_pending(hdc);
                core_machine_hdc_advance(hdc);
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_IDLE ||
                    hdc->data.sector_count != 0u || media.write_count != 5u ||
                    media.sector[1][0] != 0x33u || media.sector[1][1] != 0x44u ||
                    !core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    core_machine_hdc_irq_pending(hdc);
                if (!core_machine_hdc_write(machine, hdc_config.status_command_port, 0xecu) ||
                    hdc->data.phase != CORE_MACHINE_HDC_PHASE_PENDING_COMMAND ||
                    hdc->data.status != CORE_MACHINE_HDC_STATUS_BSY) {
                    failed |= 0x1000;
                }
                core_machine_hdc_advance(hdc);
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_DATA_READ ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    !core_machine_hdc_read(machine, hdc_config.data_port, &status) ||
                    status != 0x0040u;
                for (type_unsigned_32 index = 1u; index < 256u; ++index) {
                    failed |= !core_machine_hdc_read(machine, hdc_config.data_port, &status);
                }
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_PENDING_READ_SECTOR;
                core_machine_hdc_advance(hdc);
                failed |= hdc->data.phase != CORE_MACHINE_HDC_PHASE_IDLE ||
                    !core_machine_hdc_irq_pending(hdc);

                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_write(machine, hdc_config.status_command_port, 0x20u) ||
                    hdc->data.phase != CORE_MACHINE_HDC_PHASE_PENDING_COMMAND ||
                    !core_machine_hdc_write(machine,
                        hdc_config.alternate_status_device_control_port,
                        CORE_MACHINE_HDC_DEVICE_CONTROL_SRST) ||
                    hdc->data.phase != CORE_MACHINE_HDC_PHASE_IDLE ||
                    hdc->data.status != CORE_MACHINE_HDC_STATUS_BSY ||
                    !core_machine_hdc_write(machine,
                        hdc_config.alternate_status_device_control_port, 0u) ||
                    hdc->data.phase != CORE_MACHINE_HDC_PHASE_IDLE ||
                    hdc->data.status != (CORE_MACHINE_HDC_STATUS_DRDY |
                        CORE_MACHINE_HDC_STATUS_DSC) || core_machine_hdc_irq_pending(hdc)) {
                    failed |= 0x2000;
                }

                media.present = TYPE_FALSE;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_command(machine, &hdc_config, 0x20u) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    status != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_ERR)) {
                    failed |= 0x100;
                }
            }
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(registry);
    failed |= core_machine_hdc_test_ibm_wd1003();
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T286:S1:ATA-NIEN:PORT:FAIL bits=%x status=%02x error=%02x word=%04x\n",
            failed, status, error, word);
        return 1;
    }
    puts("M5:T286:S1:ATA-NIEN:PORT:OK");
    puts("M5:T283:S2:CORE-HDC-MEDIA:OK");
    puts("M5:T347:S3:ATA-SERVICE:OK");
    puts("M5:T479:S5:IBM-WD1003:OK");
    return 0;
}
