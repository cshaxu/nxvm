#include "type.h"

#include "core/machine/hdc.h"
#include "core/machine/machine.h"
#include "core/machine/media_interface.h"

typedef struct core_machine_hdc_fixture_media {
    uint8_t sector[512];
    uint64_t generation;
    uint32_t query_count;
    uint32_t read_count;
    uint32_t write_count;
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
    out_info->geometry.sectors_per_track = 1u;
    out_info->geometry.bytes_per_sector = 512u;
    out_info->geometry.logical_sector_count = 1u;
    return media->present ? CORE_MACHINE_MEDIA_RESULT_OK : CORE_MACHINE_MEDIA_RESULT_ABSENT;
}

static core_machine_media_result core_machine_hdc_fixture_read(C_VOID *context,
    uint64_t offset, C_VOID *buffer, uint32_t byte_count)
{
    core_machine_hdc_fixture_media *media = context;

    if (media == STD_NULL || buffer == STD_NULL || !media->present) {
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    }
    if (media->forced_read_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        return media->forced_read_result;
    }
    if (offset != 0u || byte_count != sizeof(media->sector)) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->read_count;
    STD_MEMCPY(buffer, media->sector, sizeof(media->sector));
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_hdc_fixture_write(C_VOID *context,
    uint64_t offset, const C_VOID *buffer, uint32_t byte_count)
{
    core_machine_hdc_fixture_media *media = context;

    if (media == STD_NULL || buffer == STD_NULL || !media->present) {
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    }
    if (media->read_only) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (media->forced_write_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        return media->forced_write_result;
    }
    if (offset != 0u || byte_count != sizeof(media->sector)) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->write_count;
    STD_MEMCPY(media->sector, buffer, sizeof(media->sector));
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider core_machine_hdc_fixture_provider = {
    core_machine_hdc_fixture_query,
    core_machine_hdc_fixture_read,
    core_machine_hdc_fixture_write,
    STD_NULL,
    STD_NULL
};

static C_INT core_machine_hdc_write(core_machine *machine, uint16_t port,
    uint32_t value)
{
    return core_machine_bus_write(machine, port, value) == TYPE_STATUS_OK;
}

static C_INT core_machine_hdc_read(core_machine *machine, uint16_t port,
    uint32_t *out_value)
{
    return core_machine_bus_read(machine, port, out_value) == TYPE_STATUS_OK;
}

static C_INT core_machine_hdc_program_chs(core_machine *machine,
    const core_machine_hdc_config *config)
{
    return core_machine_hdc_write(machine, config->sector_count_port, 1u) &&
        core_machine_hdc_write(machine, config->sector_number_port, 1u) &&
        core_machine_hdc_write(machine, config->cylinder_low_port, 0u) &&
        core_machine_hdc_write(machine, config->cylinder_high_port, 0u) &&
        core_machine_hdc_write(machine, config->drive_head_port, 0u);
}

static C_INT core_machine_hdc_drain(core_machine *machine,
    const core_machine_hdc_config *config, uint16_t *first_word)
{
    uint32_t word;

    for (uint32_t index = 0u; index < 256u; ++index) {
        if (!core_machine_hdc_read(machine, config->data_port, &word)) return 0;
        if (index == 0u && first_word != STD_NULL) *first_word = (uint16_t)word;
    }
    return 1;
}

static C_INT core_machine_hdc_fill(core_machine *machine,
    const core_machine_hdc_config *config, uint16_t first_word)
{
    for (uint32_t index = 0u; index < 256u; ++index) {
        if (!core_machine_hdc_write(machine, config->data_port,
                index == 0u ? first_word : 0u)) return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_hdc_config hdc_config = {
        .data_port = 0x01f0u, .error_features_port = 0x01f1u,
        .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
        .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
        .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
        .alternate_status_device_control_port = 0x03f6u,
        .irq = 14u, .lba28_supported = TYPE_TRUE
    };
    core_machine_hdc_fixture_media media = {
        .generation = 1u, .present = TYPE_TRUE,
        .forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK,
        .forced_write_result = CORE_MACHINE_MEDIA_RESULT_OK
    };
    core_machine_media_registry registry = {0};
    core_machine_hdc_topology topology = {0};
    core_machine *machine = STD_NULL;
    core_machine_hdc *hdc;
    uint32_t status = 0u;
    uint32_t error = 0u;
    uint16_t word = 0u;
    uint32_t queries_before;
    C_INT failed = 0;

    media.sector[0] = 0x34u;
    media.sector[1] = 0x12u;
    core_machine_media_registry_initialize(&registry);
    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK) failed |= 0x01;
    if (!failed) {
        hdc = &machine->hdc;
        if (hdc == STD_NULL ||
            core_machine_media_registry_bind(&registry, 1u, &media,
                &core_machine_hdc_fixture_provider) != TYPE_STATUS_OK ||
            core_machine_media_registry_freeze(&registry) != TYPE_STATUS_OK ||
            core_machine_media_registry_bind(&registry, 2u, &media,
                &core_machine_hdc_fixture_provider) != TYPE_STATUS_INVALID_STATE) {
            failed |= 0x02;
        } else {
            topology.media_registry = &registry;
            topology.media_id = 1u;
            topology.config = hdc_config;
            if (core_machine_configure_hdc(machine, &topology) != TYPE_STATUS_OK ||
                core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
                core_machine_reset(machine) != TYPE_STATUS_OK) {
                failed |= 0x04;
            } else {
                if (!core_machine_hdc_write(machine,
                        hdc_config.alternate_status_device_control_port,
                        CORE_MACHINE_HDC_DEVICE_CONTROL_NIEN) ||
                    !core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_write(machine,
                        hdc_config.status_command_port, 0x20u) ||
                    !core_machine_hdc_read(machine,
                        hdc_config.alternate_status_device_control_port, &status) ||
                    status != (CORE_MACHINE_HDC_STATUS_DRDY |
                        CORE_MACHINE_HDC_STATUS_DSC | CORE_MACHINE_HDC_STATUS_DRQ) ||
                    core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_drain(machine, &hdc_config, &word) ||
                    word != 0x1234u ||
                    !core_machine_hdc_write(machine,
                        hdc_config.alternate_status_device_control_port, 0u) ||
                    !core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_write(machine,
                        hdc_config.status_command_port, 0x20u) ||
                    !core_machine_hdc_irq_pending(hdc) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port,
                        &status) ||
                    !core_machine_hdc_drain(machine, &hdc_config, &word) ||
                    word != 0x1234u) failed |= 0x400;

                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_write(machine, hdc_config.status_command_port, 0x20u) ||
                    !core_machine_hdc_drain(machine, &hdc_config, &word) || word != 0x1234u ||
                    media.read_count != 3u) failed |= 0x08;

                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_write(machine, hdc_config.status_command_port, 0x30u) ||
                    !core_machine_hdc_fill(machine, &hdc_config, 0xa55au) ||
                    media.write_count != 1u || media.sector[0] != 0x5au ||
                    media.sector[1] != 0xa5u) failed |= 0x10;

                queries_before = media.query_count;
                ++media.generation;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_write(machine, hdc_config.status_command_port, 0x20u) ||
                    !core_machine_hdc_drain(machine, &hdc_config, &word) || word != 0xa55au ||
                    media.query_count <= queries_before) failed |= 0x20;

                media.forced_read_result = CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_write(machine, hdc_config.status_command_port, 0x20u) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    !core_machine_hdc_read(machine, hdc_config.error_features_port, &error) ||
                    status != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_ERR) ||
                    error != CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND) failed |= 0x40;
                media.forced_read_result = CORE_MACHINE_MEDIA_RESULT_OK;

                media.read_only = TYPE_TRUE;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_write(machine, hdc_config.status_command_port, 0x30u) ||
                    !core_machine_hdc_fill(machine, &hdc_config, 0xbeefu) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    !core_machine_hdc_read(machine, hdc_config.error_features_port, &error) ||
                    status != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_ERR) ||
                    error != CORE_MACHINE_HDC_ERROR_ABORT) failed |= 0x80;
                media.read_only = TYPE_FALSE;

                media.present = TYPE_FALSE;
                if (!core_machine_hdc_program_chs(machine, &hdc_config) ||
                    !core_machine_hdc_write(machine, hdc_config.status_command_port, 0x20u) ||
                    !core_machine_hdc_read(machine, hdc_config.status_command_port, &status) ||
                    status != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_ERR)) {
                    failed |= 0x100;
                }
            }
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_finalize(&registry);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T286:S1:ATA-NIEN:PORT:FAIL bits=%x status=%02x error=%02x word=%04x\n",
            failed, status, error, word);
        return 1;
    }
    puts("M5:T286:S1:ATA-NIEN:PORT:OK");
    puts("M5:T283:S2:CORE-HDC-MEDIA:OK");
    return 0;
}
