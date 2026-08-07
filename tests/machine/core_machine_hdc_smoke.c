#include "type.h"

#include "core/machine/hdc.h"
#include "core/machine/machine.h"
#include "core/machine/media_interface.h"

typedef struct core_machine_hdc_fixture_media {
    uint8_t sector[512];
} core_machine_hdc_fixture_media;

static core_machine_media_result core_machine_hdc_fixture_query(C_VOID *context,
    core_machine_media_info *out_info)
{
    if (context == STD_NULL || out_info == STD_NULL) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    STD_MEMSET(out_info, 0, sizeof(*out_info));
    out_info->present = TYPE_TRUE;
    out_info->geometry.cylinders = 1u;
    out_info->geometry.heads = 1u;
    out_info->geometry.sectors_per_track = 1u;
    out_info->geometry.bytes_per_sector = 512u;
    out_info->geometry.logical_sector_count = 1u;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_hdc_fixture_read(C_VOID *context,
    uint64_t offset, C_VOID *buffer, uint32_t byte_count)
{
    core_machine_hdc_fixture_media *media = context;
    if (media == STD_NULL || buffer == STD_NULL || offset != 0u ||
        byte_count != sizeof(media->sector)) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    STD_MEMCPY(buffer, media->sector, sizeof(media->sector));
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider core_machine_hdc_fixture_provider = {
    core_machine_hdc_fixture_query,
    core_machine_hdc_fixture_read,
    STD_NULL,
    STD_NULL,
    STD_NULL
};

C_INT main(C_VOID)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_hdc_config hdc_config = {
        .data_port = 0x01f0u,
        .error_features_port = 0x01f1u,
        .sector_count_port = 0x01f2u,
        .sector_number_port = 0x01f3u,
        .cylinder_low_port = 0x01f4u,
        .cylinder_high_port = 0x01f5u,
        .drive_head_port = 0x01f6u,
        .status_command_port = 0x01f7u,
        .alternate_status_device_control_port = 0x03f6u,
        .irq = 14u,
        .lba28_supported = TYPE_TRUE
    };
    core_machine_hdc_fixture_media media = {0};
    core_machine_media_registry registry = {0};
    const core_machine_port_provider *ports;
    core_machine *machine = STD_NULL;
    core_machine_hdc *hdc;
    uint32_t status = 0u;
    uint32_t word = 0u;
    C_INT failed = 0;

    core_machine_media_registry_initialize(&registry);
    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK) failed |= 0x01;
    if (!failed) {
        hdc = core_machine_configuration_shared_hdc_borrow(machine);
        ports = core_machine_hdc_port_provider();
        if (hdc == STD_NULL || ports == STD_NULL ||
            core_machine_media_registry_bind(&registry, 1u, &media,
                &core_machine_hdc_fixture_provider) != TYPE_STATUS_OK ||
            core_machine_media_registry_freeze(&registry) != TYPE_STATUS_OK) {
            failed |= 0x02;
        } else {
            core_machine_hdc_connect(hdc, &registry, 1u,
                core_machine_configuration_shared_pic_master_borrow(machine),
                core_machine_configuration_shared_pic_slave_borrow(machine),
                &hdc_config);
            core_machine_hdc_initialize(hdc);
            if (core_machine_install_port_provider(machine,
                    hdc_config.data_port, hdc_config.status_command_port,
                    ports, hdc) != TYPE_STATUS_OK ||
                core_machine_install_port_provider(machine,
                    hdc_config.alternate_status_device_control_port,
                    hdc_config.alternate_status_device_control_port,
                    ports, hdc) != TYPE_STATUS_OK ||
                core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
                core_machine_reset(machine) != TYPE_STATUS_OK ||
                core_machine_bus_write(machine, hdc_config.status_command_port,
                    0xecu) != TYPE_STATUS_OK ||
                core_machine_bus_read(machine,
                    hdc_config.alternate_status_device_control_port, &status) !=
                    TYPE_STATUS_OK ||
                status != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC |
                    CORE_MACHINE_HDC_STATUS_DRQ) ||
                !core_machine_hdc_irq_pending(&machine->shared_hdc) ||
                core_machine_bus_read(machine, hdc_config.data_port, &word) !=
                    TYPE_STATUS_OK || word != 0x0040u ||
                core_machine_bus_read(machine, hdc_config.status_command_port,
                    &status) != TYPE_STATUS_OK ||
                core_machine_hdc_irq_pending(&machine->shared_hdc)) {
                failed |= 0x04;
            }
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_finalize(&registry);
    if (failed) return 1;
    puts("M5:T278:S3:CORE-HDC:OK");
    return 0;
}
