#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/fdc.h"
#include "core/machine/machine.h"
#include "core/machine/media_interface.h"
#include "core/machine/port.h"

typedef struct core_machine_fdc_topology_media {
    uint8_t byte;
    uint32_t read_count;
} core_machine_fdc_topology_media;

static core_machine_media_result core_machine_fdc_topology_query(C_VOID *context,
    core_machine_media_info *out_info)
{
    if (context == STD_NULL || out_info == STD_NULL) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
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

static core_machine_media_result core_machine_fdc_topology_read(C_VOID *context,
    uint64_t offset, C_VOID *buffer, uint32_t byte_count)
{
    core_machine_fdc_topology_media *media = context;

    if (media == STD_NULL || buffer == STD_NULL || offset >= 512u || byte_count != 1u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    *(uint8_t *)buffer = media->byte;
    ++media->read_count;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider core_machine_fdc_topology_provider = {
    core_machine_fdc_topology_query,
    core_machine_fdc_topology_read,
    STD_NULL,
    STD_NULL,
    STD_NULL
};

static C_VOID core_machine_fdc_topology_command(t_port *port, const uint8_t *bytes,
    STD_SIZE_T count)
{
    STD_SIZE_T index;

    for (index = 0u; index < count; ++index) {
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    }
}

static C_INT core_machine_fdc_topology_result(t_port *port, uint8_t *result,
    STD_SIZE_T count)
{
    STD_SIZE_T index;

    for (index = 0u; index < count; ++index) {
        result[index] = (uint8_t)core_machine_port_read(port, 0x03f5u);
    }
    return (core_machine_port_read(port, 0x03f4u) & (VFDC_MSR_CB | VFDC_MSR_DIO)) == 0u;
}

static C_INT core_machine_fdc_topology_read_sector(t_port *port, uint8_t unit,
    uint8_t expected, uint8_t *result)
{
    const uint8_t command[] = {0xe6u, unit, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu};
    uint32_t index;

    core_machine_fdc_topology_command(port, command, sizeof(command));
    if (core_machine_port_read(port, 0x03f5u) != expected) return 0;
    for (index = 1u; index < 512u; ++index) {
        (C_VOID)core_machine_port_read(port, 0x03f5u);
    }
    return core_machine_fdc_topology_result(port, result, 7u) &&
        result[0] == core_machine_fdc_ST0_NORMAL && result[1] == 0u;
}

int main(C_VOID)
{
    static const uint8_t specify_non_dma[] = {0x03u, 0xdfu, 0x03u};
    uint8_t sense_drive[] = {0x04u, 0u};
    static const uint8_t read_absent[] = {0xe6u, 2u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu};
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
        {11u, 12u, CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}
    };
    core_machine_fdc_topology_media drive0 = {.byte = 0xa1u};
    core_machine_fdc_topology_media drive1 = {.byte = 0xb2u};
    core_machine_media_registry media = {0};
    core_machine_dma_request_binding dma_request = {0};
    core_machine *machine = STD_NULL;
    core_machine_fdc *fdc;
    t_port *port;
    uint8_t result[7] = {0};
    C_INT failed = 0;

    core_machine_media_registry_initialize(&media);
    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK) failed |= 0x01;
    if (!failed) {
        fdc = core_machine_configuration_fdc_borrow(machine);
        port = core_machine_configuration_port_borrow(machine);
        if (fdc == STD_NULL || port == STD_NULL ||
            core_machine_media_registry_bind(&media, 11u, &drive0,
                &core_machine_fdc_topology_provider) != TYPE_STATUS_OK ||
            core_machine_media_registry_bind(&media, 12u, &drive1,
                &core_machine_fdc_topology_provider) != TYPE_STATUS_OK ||
            core_machine_media_registry_freeze(&media) != TYPE_STATUS_OK ||
            core_machine_media_registry_bind(&media, 13u, &drive0,
                &core_machine_fdc_topology_provider) != TYPE_STATUS_INVALID_STATE ||
            core_machine_dma_bind_channel(
                core_machine_configuration_shared_dma_latch_borrow(machine),
                core_machine_configuration_shared_dma_primary_borrow(machine),
                core_machine_configuration_shared_dma_secondary_borrow(machine),
                fdc_config.dma_channel, core_machine_fdc_dma_provider(), fdc,
                &dma_request) != TYPE_STATUS_OK) {
            failed |= 0x02;
        } else {
            core_machine_fdc_connect(fdc, &media, &drives, &dma_request,
                core_machine_configuration_shared_pic_master_borrow(machine),
                core_machine_configuration_shared_pic_slave_borrow(machine), port,
                &fdc_config);
            core_machine_fdc_initialize(fdc);
            if (core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
                core_machine_reset(machine) != TYPE_STATUS_OK) {
                failed |= 0x04;
            } else {
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                core_machine_fdc_topology_command(port, specify_non_dma,
                    sizeof(specify_non_dma));
                core_machine_fdc_topology_command(port, sense_drive,
                    sizeof(sense_drive));
                failed |= !core_machine_fdc_topology_result(port, result, 1u) ||
                    result[0] != 0x30u ||
                    !core_machine_fdc_topology_read_sector(port, 0u, 0xa1u, result) ||
                    drive0.read_count != 512u || drive1.read_count != 0u;

                core_machine_port_write(port, 0x03f2u, 0x2du);
                sense_drive[1] = 1u;
                core_machine_fdc_topology_command(port, sense_drive,
                    sizeof(sense_drive));
                failed |= !core_machine_fdc_topology_result(port, result, 1u) ||
                    result[0] != 0x31u ||
                    !core_machine_fdc_topology_read_sector(port, 1u, 0xb2u, result) ||
                    drive0.read_count != 512u || drive1.read_count != 512u;

                core_machine_fdc_topology_command(port,
                    (const uint8_t[]){0xe6u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu}, 9u);
                failed |= !core_machine_fdc_topology_result(port, result, 7u) ||
                    result[0] != core_machine_fdc_ST0_ABNORMAL || result[1] != 0x04u ||
                    drive0.read_count != 512u || drive1.read_count != 512u;

                core_machine_port_write(port, 0x03f2u, 0x4eu);
                sense_drive[1] = 2u;
                core_machine_fdc_topology_command(port, sense_drive,
                    sizeof(sense_drive));
                failed |= !core_machine_fdc_topology_result(port, result, 1u) ||
                    result[0] != 0x12u;
                core_machine_fdc_topology_command(port, read_absent, sizeof(read_absent));
                failed |= !core_machine_fdc_topology_result(port, result, 7u) ||
                    result[0] != core_machine_fdc_ST0_ABNORMAL || result[1] != 0x04u ||
                    drive0.read_count != 512u || drive1.read_count != 512u;
            }
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_finalize(&media);
    if (failed) return 1;
    puts("M5:T290:S1:FDC:PORT:OK");
    return 0;
}
