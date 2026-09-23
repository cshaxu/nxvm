#include "lib/types/types_interface.h"
#include "type.h"

#include "app-nxvm/devices/dma.h"
#include "app-nxvm/devices/fdc.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/devices/port.h"

typedef struct core_machine_fdc_topology_media {
    lib_u8 byte;
    lib_u32 read_count;
} core_machine_fdc_topology_media;

static core_machine_media_result core_machine_fdc_topology_query(C_VOID *context,
    core_machine_media_info *out_info)
{
    if (context == LIB_NULL || out_info == LIB_NULL) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    lib_memory_set(out_info, 0, sizeof(*out_info));
    out_info->present = LIB_TRUE;
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
    lib_u64 offset, C_VOID *buffer, lib_u32 byte_count)
{
    core_machine_fdc_topology_media *media = context;

    if (media == LIB_NULL || buffer == LIB_NULL || offset >= 512u || byte_count != 1u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    *(lib_u8 *)buffer = media->byte;
    ++media->read_count;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider core_machine_fdc_topology_provider = {
    core_machine_fdc_topology_query,
    core_machine_fdc_topology_read,
    LIB_NULL,
    LIB_NULL,
    LIB_NULL,
    LIB_NULL,
    LIB_NULL
};

static C_VOID core_machine_fdc_topology_command(core_machine_fdc *fdc, t_port *port,
    const lib_u8 *bytes, lib_size count)
{
    lib_size index;

    for (index = 0u; index < count; ++index) {
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    }
    core_machine_fdc_advance(fdc);
}

static C_INT core_machine_fdc_topology_result(core_machine_fdc *fdc, t_port *port,
    lib_u8 *result, lib_size count)
{
    lib_size index;

    core_machine_fdc_advance(fdc);
    for (index = 0u; index < count; ++index) {
        result[index] = (lib_u8)core_machine_port_read(port, 0x03f5u);
    }
    return (core_machine_port_read(port, 0x03f4u) & (VFDC_MSR_CB | VFDC_MSR_DIO)) == 0u;
}

static C_INT core_machine_fdc_topology_read_sector(core_machine_fdc *fdc, t_port *port,
    lib_u8 unit, lib_u8 expected, lib_u8 *result)
{
    const lib_u8 command[] = {0xe6u, unit, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu};
    lib_u32 index;

    core_machine_fdc_topology_command(fdc, port, command, sizeof(command));
    if (core_machine_port_read(port, 0x03f5u) != expected) return 0;
    for (index = 1u; index < 512u; ++index) {
        core_machine_fdc_advance_at(fdc, fdc->data.elapsed_ticks + 128u);
        (C_VOID)core_machine_port_read(port, 0x03f5u);
    }
    return core_machine_fdc_topology_result(fdc, port, result, 7u) &&
        result[0] == core_machine_fdc_ST0_NORMAL && result[1] == 0u;
}

int main(C_VOID)
{
    static const lib_u8 specify_non_dma[] = {0x03u, 0xdfu, 0x03u};
    lib_u8 sense_drive[] = {0x04u, 0u};
    static const lib_u8 read_absent[] = {0xe6u, 2u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu};
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
        .clock_ticks_per_second = 8000000u
    };
    const core_machine_fdc_drive_bindings drives = {
        {11u, 12u, CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}, 0x03u, 0x03u,
        {0u, 0u, 0u, 0u}, 0u
    };
    const core_machine_dma_wiring dma_wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_fdc_topology_media drive0 = {.byte = 0xa1u};
    core_machine_fdc_topology_media drive1 = {.byte = 0xb2u};
    core_machine_media_registry *media = LIB_NULL;
    core_machine_dma_request_binding dma_request = {0};
    core_machine_fdc_topology topology = {0};
    core_machine *machine = LIB_NULL;
    core_machine_fdc *fdc = LIB_NULL;
    lib_u8 diagnostic_phase = 0u;
    lib_u8 reset_drive;
    t_port *port = LIB_NULL;
    lib_u8 result[7] = {0};
    C_INT failed = 0;

    if (core_machine_media_registry_create(&media) != TYPE_STATUS_OK ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK) failed |= 0x01;
    if (!failed) {
        fdc = &machine->fdc;
        port = &machine->executor_port;
        if (fdc == LIB_NULL || port == LIB_NULL ||
            core_machine_media_registry_bind(media, 11u, &drive0,
                &core_machine_fdc_topology_provider) != TYPE_STATUS_OK ||
            core_machine_media_registry_bind(media, 12u, &drive1,
                &core_machine_fdc_topology_provider) != TYPE_STATUS_OK ||
            core_machine_media_registry_freeze(media) != TYPE_STATUS_OK ||
            core_machine_media_registry_bind(media, 13u, &drive0,
                &core_machine_fdc_topology_provider) != TYPE_STATUS_INVALID_STATE ||
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
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                failed |= fdc->connect.irq_source.asserted ? 0x08 : 0;
                core_machine_fdc_advance_at(fdc, fdc->data.reset_due_tick);
                failed |= !fdc->connect.irq_source.asserted ? 0x08 : 0;
                for (reset_drive = 0u; reset_drive < CORE_MACHINE_FDC_DRIVE_COUNT;
                    ++reset_drive) {
                    core_machine_fdc_topology_command(fdc, port,
                        (const lib_u8[]){0x08u}, 1u);
                    failed |= (!core_machine_fdc_topology_result(fdc, port, result, 2u) ||
                        result[0] != (core_machine_fdc_ST0_READY_CHANGE | reset_drive) ||
                        result[1] != 0u || fdc->connect.irq_source.asserted) ? 0x08 : 0;
                }
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                core_machine_fdc_topology_command(fdc, port,
                    (const lib_u8[]){0x08u}, 1u);
                failed |= (!core_machine_fdc_topology_result(fdc, port, result, 2u) ||
                    result[0] != 0x80u || result[1] != 0u ||
                    fdc->connect.irq_source.asserted) ? 0x08 : 0;
                core_machine_fdc_topology_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));
                core_machine_fdc_topology_command(fdc, port, sense_drive,
                    sizeof(sense_drive));
                failed |= (!core_machine_fdc_topology_result(fdc, port, result, 1u) ||
                    result[0] != 0x38u ||
                    !core_machine_fdc_topology_read_sector(fdc, port, 0u, 0xa1u, result) ||
                    drive0.read_count != 512u || drive1.read_count != 0u) ? 0x10 : 0;

                core_machine_port_write(port, 0x03f2u, 0x2du);
                sense_drive[1] = 1u;
                core_machine_fdc_topology_command(fdc, port, sense_drive,
                    sizeof(sense_drive));
                failed |= (!core_machine_fdc_topology_result(fdc, port, result, 1u) ||
                    result[0] != 0x39u ||
                    !core_machine_fdc_topology_read_sector(fdc, port, 1u, 0xb2u, result) ||
                    drive0.read_count != 512u || drive1.read_count != 512u) ? 0x20 : 0;

                core_machine_fdc_topology_command(fdc, port,
                    (const lib_u8[]){0xe6u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu}, 9u);
                failed |= (!core_machine_fdc_topology_result(fdc, port, result, 7u) ||
                    result[0] != core_machine_fdc_ST0_ABNORMAL || result[1] != 0x04u ||
                    drive0.read_count != 512u || drive1.read_count != 512u) ? 0x40 : 0;

                core_machine_port_write(port, 0x03f2u, 0x4eu);
                sense_drive[1] = 2u;
                core_machine_fdc_topology_command(fdc, port, sense_drive,
                    sizeof(sense_drive));
                failed |= (!core_machine_fdc_topology_result(fdc, port, result, 1u) ||
                    result[0] != 0x22u) ? 0x80 : 0;
                core_machine_fdc_topology_command(fdc, port,
                    (const lib_u8[]){0x07u, 2u}, 2u);
                core_machine_fdc_advance_at(fdc, fdc->data.seek_due_tick[2u]);
                core_machine_fdc_topology_command(fdc, port,
                    (const lib_u8[]){0x08u}, 1u);
                failed |= (!core_machine_fdc_topology_result(fdc, port, result, 2u) ||
                    result[0] != (core_machine_fdc_ST0_ABNORMAL |
                        VFDC_ST0_SEEK_END | VFDC_ST0_EQUIPMENT_CHECK | 2u) ||
                    result[1] != 0u) ? 0x100 : 0;
                core_machine_fdc_topology_command(fdc, port,
                    (const lib_u8[]){0x0fu, 2u, 1u}, 3u);
                core_machine_fdc_advance_at(fdc, fdc->data.seek_due_tick[2u]);
                core_machine_fdc_topology_command(fdc, port,
                    (const lib_u8[]){0x08u}, 1u);
                failed |= (!core_machine_fdc_topology_result(fdc, port, result, 2u) ||
                    result[0] != (core_machine_fdc_ST0_NORMAL | 2u) ||
                    result[1] != 1u) ? 0x200 : 0;
                core_machine_fdc_topology_command(fdc, port, read_absent, sizeof(read_absent));
                failed |= (!core_machine_fdc_topology_result(fdc, port, result, 7u) ||
                    result[0] != core_machine_fdc_ST0_ABNORMAL || result[1] != 0x04u ||
                    drive0.read_count != 512u || drive1.read_count != 512u) ? 0x400 : 0;
            }
        }
    }
    if (fdc != LIB_NULL) diagnostic_phase = fdc->data.phase;
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    if (failed) {
        STD_FPRINTF(stderr, "M5:T380:S2:FDC-TOPOLOGY:FAIL:%x:reads=%u,%u:phase=%u\n",
            failed, drive0.read_count, drive1.read_count,
            diagnostic_phase);
        return 1;
    }
    puts("M5:T290:S1:FDC:PORT:OK");
    return 0;
}
