#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/fdc.h"
#include "core/machine/machine.h"
#include "core/machine/media_interface.h"
#include "core/machine/port.h"

typedef struct core_machine_fdc_change_media {
    type_unsigned_64 generation;
    type_bool present;
} core_machine_fdc_change_media;

static C_VOID core_machine_fdc_change_require(C_INT *failed,
    C_INT *first_failure, C_INT step, C_INT condition)
{
    if (!condition) return;
    *failed = 1;
    if (*first_failure == 0) *first_failure = step;
}

static core_machine_media_result core_machine_fdc_change_query(C_VOID *context,
    core_machine_media_info *out_info)
{
    core_machine_fdc_change_media *media = context;

    if (media == STD_NULL || out_info == STD_NULL) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    STD_MEMSET(out_info, 0, sizeof(*out_info));
    out_info->generation = media->generation;
    out_info->present = media->present;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_REMOVABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
        CORE_MACHINE_MEDIA_CAPABILITY_CHANGE_DETECTABLE;
    out_info->geometry.cylinders = 1u;
    out_info->geometry.heads = 1u;
    out_info->geometry.sectors_per_track = 1u;
    out_info->geometry.bytes_per_sector = 512u;
    out_info->geometry.logical_sector_count = 1u;
    return media->present ? CORE_MACHINE_MEDIA_RESULT_OK : CORE_MACHINE_MEDIA_RESULT_ABSENT;
}

static core_machine_media_result core_machine_fdc_change_read(C_VOID *context,
    type_unsigned_64 offset, C_VOID *buffer, type_unsigned_32 byte_count)
{
    core_machine_fdc_change_media *media = context;

    if (media == STD_NULL || buffer == STD_NULL || !media->present) {
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    }
    if (offset >= 512u || byte_count != 1u) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    *(type_unsigned_8 *)buffer = 0x5au;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider core_machine_fdc_change_provider = {
    core_machine_fdc_change_query,
    core_machine_fdc_change_read,
    STD_NULL,
    STD_NULL,
    STD_NULL,
    STD_NULL,
    STD_NULL
};

static C_VOID core_machine_fdc_change_command(core_machine_fdc *fdc, t_port *port,
    const type_unsigned_8 *bytes, STD_SIZE_T count)
{
    STD_SIZE_T index;

    for (index = 0u; index < count; ++index) {
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    }
    core_machine_fdc_advance(fdc);
}

static C_VOID core_machine_fdc_change_ack_irq(core_machine_fdc *fdc, t_port *port)
{
    for (type_unsigned_8 drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        if (fdc->data.seek_pending[drive]) {
            core_machine_fdc_advance_at(fdc, fdc->data.seek_due_tick[drive]);
        }
    }
    core_machine_fdc_change_command(fdc, port, (const type_unsigned_8[]){0x08u}, 1u);
    (C_VOID)core_machine_port_read(port, 0x03f5u);
    (C_VOID)core_machine_port_read(port, 0x03f5u);
}

static C_VOID core_machine_fdc_change_drain_reset(core_machine_fdc *fdc, t_port *port)
{
    type_unsigned_8 drive;

    for (drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        core_machine_fdc_change_ack_irq(fdc, port);
    }
}

int main(C_VOID)
{
    static const type_unsigned_8 recalibrate_0[] = {0x07u, 0x00u};
    static const type_unsigned_8 recalibrate_1[] = {0x07u, 0x01u};
    static const type_unsigned_8 sense_1[] = {0x04u, 0x01u};
    static const type_unsigned_8 specify_dma[] = {0x03u, 0xdfu, 0x02u};
    static const type_unsigned_8 read_0[] = {0xe6u, 0x00u, 0x00u, 0x00u, 0x01u,
        0x02u, 0x01u, 0x1bu, 0xffu};
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
        {21u, 22u, CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}
    };
    const core_machine_dma_wiring dma_wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_fdc_change_media drive0 = {.generation = 10u, .present = TYPE_TRUE};
    core_machine_fdc_change_media drive1 = {.generation = 30u, .present = TYPE_TRUE};
    core_machine_media_registry *media = STD_NULL;
    core_machine_dma_request_binding dma_request = {0};
    core_machine_fdc_topology topology = {0};
    core_machine *machine = STD_NULL;
    core_machine_fdc *fdc;
    t_dma *dma;
    t_port *port;
    type_unsigned_8 status;
    C_INT failed = 0;
    C_INT first_failure = 0;

    if (core_machine_media_registry_create(&media) != TYPE_STATUS_OK ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK) failed = 1;
    if (!failed) {
        fdc = &machine->fdc;
        dma = &machine->shared_dma_primary;
        port = &machine->executor_port;
        if (fdc == STD_NULL || dma == STD_NULL || port == STD_NULL ||
            core_machine_media_registry_bind(media, 21u, &drive0,
                &core_machine_fdc_change_provider) != TYPE_STATUS_OK ||
            core_machine_media_registry_bind(media, 22u, &drive1,
                &core_machine_fdc_change_provider) != TYPE_STATUS_OK ||
            core_machine_media_registry_freeze(media) != TYPE_STATUS_OK ||
            core_machine_configure_dma(machine, &dma_wiring, &dma_request) !=
                TYPE_STATUS_OK) {
            failed = 1;
        } else {
            topology.media_registry = media;
            topology.drives = drives;
            topology.dma_request = dma_request;
            topology.config = fdc_config;
            if (core_machine_configure_fdc(machine, &topology) != TYPE_STATUS_OK ||
                core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
                core_machine_reset(machine) != TYPE_STATUS_OK) {
                failed = 1;
            } else {
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                core_machine_fdc_change_drain_reset(fdc, port);
                core_machine_fdc_change_command(fdc, port, recalibrate_0,
                    sizeof(recalibrate_0));
                core_machine_fdc_change_ack_irq(fdc, port);
                core_machine_port_write(port, 0x03f2u, 0x2du);
                core_machine_fdc_change_command(fdc, port, recalibrate_1,
                    sizeof(recalibrate_1));
                core_machine_fdc_change_ack_irq(fdc, port);

                core_machine_fdc_change_command(fdc, port, specify_dma,
                    sizeof(specify_dma));
                drive1.present = TYPE_FALSE;
                (C_VOID)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 1,
                    !fdc->data.flagINTR || !fdc->connect.irq_source.asserted);
                core_machine_fdc_change_command(fdc, port, (const type_unsigned_8[]){0x08u}, 1u);
                status = (type_unsigned_8)core_machine_port_read(port, 0x03f5u);
                core_machine_fdc_change_require(&failed, &first_failure, 2,
                    status != (core_machine_fdc_ST0_READY_CHANGE |
                    core_machine_fdc_ST0_NOT_READY | 1u) ||
                    core_machine_port_read(port, 0x03f5u) != 0u || fdc->data.flagINTR ||
                    fdc->connect.irq_source.asserted);
                drive1.present = TYPE_TRUE;
                (C_VOID)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 3,
                    !fdc->data.flagINTR || !fdc->connect.irq_source.asserted);
                core_machine_fdc_change_ack_irq(fdc, port);

                ++drive0.generation;
                (C_VOID)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 4,
                    fdc->data.flagINTR || fdc->connect.irq_source.asserted);
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                (C_VOID)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 5,
                    (core_machine_port_read(port, 0x03f7u) & VFDC_DIR_DC) == 0u);
                core_machine_fdc_change_command(fdc, port, recalibrate_0,
                    sizeof(recalibrate_0));
                core_machine_fdc_change_ack_irq(fdc, port);
                (C_VOID)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 6,
                    (core_machine_port_read(port, 0x03f7u) & VFDC_DIR_DC) != 0u);

                ++drive1.generation;
                (C_VOID)core_machine_port_read(port, 0x03f7u);
                core_machine_port_write(port, 0x03f2u, 0x2du);
                (C_VOID)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 7,
                    (core_machine_port_read(port, 0x03f7u) & VFDC_DIR_DC) == 0u);
                drive1.present = TYPE_FALSE;
                ++drive1.generation;
                (C_VOID)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 8,
                    !fdc->data.flagINTR || !fdc->connect.irq_source.asserted);
                core_machine_fdc_change_ack_irq(fdc, port);
                status = 0u;
                core_machine_fdc_change_command(fdc, port, sense_1, sizeof(sense_1));
                status = (type_unsigned_8)core_machine_port_read(port, 0x03f5u);
                core_machine_fdc_change_require(&failed, &first_failure, 9,
                    (core_machine_port_read(port, 0x03f7u) & VFDC_DIR_DC) == 0u ||
                    status != 0x11u);

                drive1.present = TYPE_TRUE;
                (C_VOID)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 10,
                    !fdc->data.flagINTR || !fdc->connect.irq_source.asserted);
                core_machine_port_write(port, 0x03f2u, 0x00u);
                core_machine_fdc_change_require(&failed, &first_failure, 11,
                    fdc->data.flagINTR || fdc->connect.irq_source.asserted ||
                    fdc->data.phase != core_machine_fdc_PHASE_COMMAND);
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                core_machine_fdc_change_drain_reset(fdc, port);
                core_machine_fdc_change_command(fdc, port, specify_dma, sizeof(specify_dma));
                core_machine_fdc_change_command(fdc, port, read_0, sizeof(read_0));
                core_machine_fdc_change_require(&failed, &first_failure, 12,
                    (dma->data.status & VDMA_STATUS_DRQ(2u)) == 0u);
                core_machine_port_write(port, 0x03f2u, 0x0cu);
                core_machine_fdc_change_require(&failed, &first_failure, 13,
                    (dma->data.status & VDMA_STATUS_DRQ(2u)) != 0u ||
                    fdc->data.phase != core_machine_fdc_PHASE_COMMAND);
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                core_machine_fdc_change_drain_reset(fdc, port);
                core_machine_fdc_change_command(fdc, port, read_0, sizeof(read_0));
                core_machine_fdc_change_require(&failed, &first_failure, 14,
                    (dma->data.status & VDMA_STATUS_DRQ(2u)) == 0u);
                core_machine_port_write(port, 0x03f2u, 0x00u);
                core_machine_fdc_change_require(&failed, &first_failure, 15,
                    (dma->data.status & VDMA_STATUS_DRQ(2u)) != 0u ||
                    fdc->data.flagINTR || fdc->connect.irq_source.asserted ||
                    fdc->data.phase != core_machine_fdc_PHASE_COMMAND);
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                core_machine_fdc_change_drain_reset(fdc, port);
                core_machine_fdc_change_command(fdc, port, recalibrate_0,
                    sizeof(recalibrate_0));
                core_machine_fdc_advance_at(fdc, fdc->data.seek_due_tick[0u]);
                core_machine_fdc_change_require(&failed, &first_failure, 16,
                    !fdc->data.flagINTR || !fdc->connect.irq_source.asserted);
                core_machine_port_write(port, 0x03f2u, 0x00u);
                core_machine_fdc_change_require(&failed, &first_failure, 17,
                    fdc->data.flagINTR || fdc->connect.irq_source.asserted);
            }
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    if (failed) {
        STD_FPRINTF(stderr, "M5:T380:S2:FDC-MEDIA-CHANGE:FAIL:step=%d\n",
            first_failure);
        return 1;
    }
    puts("M5:T291:S1:FDC:PORT:OK");
    return 0;
}
