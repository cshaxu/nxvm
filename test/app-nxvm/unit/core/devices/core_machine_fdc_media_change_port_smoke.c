#include "support/fdc_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/dma_bus.h"
#include "app-nxvm/devices/fdc.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/devices/port.h"

typedef struct core_machine_fdc_change_media {
    lib_u64 generation;
    lib_u8 present;
} core_machine_fdc_change_media;

static void core_machine_fdc_change_require(lib_i32 *failed,
    lib_i32 *first_failure, lib_i32 step, lib_i32 condition)
{
    if (!condition) return;
    *failed = 1;
    if (*first_failure == 0) *first_failure = step;
}

static core_machine_media_result core_machine_fdc_change_query(void *context,
    core_machine_media_info *out_info)
{
    core_machine_fdc_change_media *media = context;

    if (media == LIB_NULL || out_info == LIB_NULL) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    lib_memory_set(out_info, 0, sizeof(*out_info));
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

static core_machine_media_result core_machine_fdc_change_read(void *context,
    lib_u64 offset, void *buffer, lib_u32 byte_count)
{
    core_machine_fdc_change_media *media = context;

    if (media == LIB_NULL || buffer == LIB_NULL || !media->present) {
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    }
    if (offset >= 512u || byte_count != 1u) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    *(lib_u8 *)buffer = 0x5au;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider core_machine_fdc_change_provider = {
    core_machine_fdc_change_query,
    core_machine_fdc_change_read,
    LIB_NULL,
    LIB_NULL,
    LIB_NULL,
    LIB_NULL,
    LIB_NULL
};

static void core_machine_fdc_change_command(core_machine_fdc *fdc, t_port *port,
    const lib_u8 *bytes, lib_size count)
{
    lib_size index;

    for (index = 0u; index < count; ++index) {
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    }
    test_fdc_advance(fdc);
}

static lib_bool core_machine_fdc_change_ack_irq(core_machine_fdc *fdc, t_port *port)
{
    if (!test_fdc_finish_seeks(fdc)) return LIB_FALSE;
    core_machine_fdc_change_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
    (void)core_machine_port_read(port, 0x03f5u);
    (void)core_machine_port_read(port, 0x03f5u);
    return LIB_TRUE;
}

static lib_bool core_machine_fdc_change_drain_reset(core_machine_fdc *fdc, t_port *port)
{
    lib_u8 drive;

    for (drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        if (!core_machine_fdc_change_ack_irq(fdc, port)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

int main(void)
{
    static const lib_u8 recalibrate_0[] = {0x07u, 0x00u};
    static const lib_u8 recalibrate_1[] = {0x07u, 0x01u};
    static const lib_u8 sense_1[] = {0x04u, 0x01u};
    static const lib_u8 specify_dma[] = {0x03u, 0xdfu, 0x02u};
    static const lib_u8 read_0[] = {0xe6u, 0x00u, 0x00u, 0x00u, 0x01u,
        0x02u, 0x01u, 0x1bu, 0xffu};
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_fdc_config fdc_config = {
        .dor_port = 0x03f2u, .status_port = 0x03f4u, .data_port = 0x03f5u,
        .direction_port = 0x03f7u, .control_port = 0x03f7u,
        .irq = 6u, .dma_channel = 2u, .ready_mask = 0x0fu
    };
    const core_machine_fdc_drive_bindings drives = {
        {21u, 22u, CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}, 0x03u, 0x03u,
        {0u, 0u, 0u, 0u}, 0u, {0}
    };
    const core_machine_dma_wiring dma_wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_fdc_change_media drive0 = {.generation = 10u, .present = LIB_FALSE};
    core_machine_fdc_change_media drive1 = {.generation = 30u, .present = LIB_TRUE};
    core_machine_media_registry *media = LIB_NULL;
    core_machine_dma_request_binding dma_request = {0};
    core_machine_fdc_topology topology = {0};
    core_machine *machine = LIB_NULL;
    core_machine_fdc *fdc;
    t_dma *dma;
    t_port *port;
    lib_u8 status;
    lib_i32 failed = 0;
    lib_i32 first_failure = 0;

    if (core_machine_media_registry_create(&media) != LIB_STATUS_OK ||
        core_machine_create(&config, &machine) != LIB_STATUS_OK) failed = 1;
    if (!failed) {
        fdc = &machine->board->fdc;
        dma = &machine->board->shared_dma_primary;
        port = &machine->executor_port;
        if (fdc == LIB_NULL || dma == LIB_NULL || port == LIB_NULL ||
            core_machine_media_registry_bind(media, 21u, &drive0,
                &core_machine_fdc_change_provider) != LIB_STATUS_OK ||
            core_machine_media_registry_bind(media, 22u, &drive1,
                &core_machine_fdc_change_provider) != LIB_STATUS_OK ||
            core_machine_media_registry_freeze(media) != LIB_STATUS_OK ||
            core_machine_configure_dma(machine, &dma_wiring, &dma_request) !=
                LIB_STATUS_OK) {
            failed = 1;
        } else {
            topology.media_registry = media;
            topology.drives = drives;
            topology.dma_request = dma_request;
            topology.config = fdc_config;
            if (core_machine_configure_fdc(machine, &topology) != LIB_STATUS_OK ||
                core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                (drive0.present = LIB_TRUE, core_machine_reset(machine)) != LIB_STATUS_OK) {
                failed = 1;
            } else {
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                core_machine_fdc_change_require(&failed, &first_failure, 1,
                    (core_machine_port_read(port, 0x03f7u) & VFDC_DIR_DC) != 0u);
                failed |= !core_machine_fdc_change_drain_reset(fdc, port);
                core_machine_fdc_change_command(fdc, port, recalibrate_0,
                    sizeof(recalibrate_0));
                failed |= !core_machine_fdc_change_ack_irq(fdc, port);
                core_machine_port_write(port, 0x03f2u, 0x2du);
                core_machine_fdc_change_command(fdc, port, recalibrate_1,
                    sizeof(recalibrate_1));
                failed |= !core_machine_fdc_change_ack_irq(fdc, port);

                core_machine_fdc_change_command(fdc, port, specify_dma,
                    sizeof(specify_dma));
                /* These boards tie READY high: media changes affect DIR,
                 * not the controller's independent READY-change interrupt. */
                drive1.present = LIB_FALSE;
                (void)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 2,
                    !test_fdc_interrupt_matches(fdc, LIB_FALSE) ||
                    (core_machine_port_read(port, 0x03f7u) & VFDC_DIR_DC) == 0u);
                core_machine_fdc_change_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
                status = (lib_u8)core_machine_port_read(port, 0x03f5u);
                core_machine_fdc_change_require(&failed, &first_failure, 3,
                    status != 0x80u ||
                    (core_machine_port_read(port, 0x03f4u) &
                        (TEST_FDC_MSR_CB | TEST_FDC_MSR_DIO)) != 0u ||
                    !test_fdc_interrupt_matches(fdc, LIB_FALSE));
                drive1.present = LIB_TRUE;
                (void)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 4,
                    !test_fdc_interrupt_matches(fdc, LIB_FALSE));
                failed |= !core_machine_fdc_change_ack_irq(fdc, port);

                ++drive0.generation;
                (void)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 5,
                    !test_fdc_interrupt_matches(fdc, LIB_FALSE));
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                (void)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 6,
                    (core_machine_port_read(port, 0x03f7u) & VFDC_DIR_DC) == 0u);
                /* A real step clears the drive change latch; a no-motion
                 * recalibration at Track0 cannot do so. */
                core_machine_fdc_change_command(fdc, port,
                    (const lib_u8[]){0x0fu, 0u, 1u}, 3u);
                failed |= !core_machine_fdc_change_ack_irq(fdc, port);
                core_machine_fdc_change_command(fdc, port, recalibrate_0,
                    sizeof(recalibrate_0));
                failed |= !core_machine_fdc_change_ack_irq(fdc, port);
                (void)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 7,
                    (core_machine_port_read(port, 0x03f7u) & VFDC_DIR_DC) != 0u);

                ++drive1.generation;
                (void)core_machine_port_read(port, 0x03f7u);
                core_machine_port_write(port, 0x03f2u, 0x2du);
                (void)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 8,
                    (core_machine_port_read(port, 0x03f7u) & VFDC_DIR_DC) == 0u);
                drive1.present = LIB_FALSE;
                ++drive1.generation;
                (void)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 9,
                    !test_fdc_interrupt_matches(fdc, LIB_FALSE));
                failed |= !core_machine_fdc_change_ack_irq(fdc, port);
                status = 0u;
                core_machine_fdc_change_command(fdc, port, sense_1, sizeof(sense_1));
                status = (lib_u8)core_machine_port_read(port, 0x03f5u);
                core_machine_fdc_change_require(&failed, &first_failure, 10,
                    (core_machine_port_read(port, 0x03f7u) & VFDC_DIR_DC) == 0u ||
                    status != 0x39u);

                drive1.present = LIB_TRUE;
                (void)core_machine_port_read(port, 0x03f7u);
                core_machine_fdc_change_require(&failed, &first_failure, 11,
                    !test_fdc_interrupt_matches(fdc, LIB_FALSE));
                core_machine_port_write(port, 0x03f2u, 0x00u);
                core_machine_fdc_change_require(&failed, &first_failure, 12,
                    !test_fdc_interrupt_matches(fdc, LIB_FALSE) ||
                    core_machine_port_read(port, 0x03f4u) != TEST_FDC_MSR_RQM);
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                failed |= !core_machine_fdc_change_drain_reset(fdc, port);
                core_machine_fdc_change_command(fdc, port, specify_dma, sizeof(specify_dma));
                core_machine_fdc_change_command(fdc, port, read_0, sizeof(read_0));
                core_machine_fdc_change_require(&failed, &first_failure, 13,
                    (core_machine_port_read(port, 8u) & 0x40u) == 0u);
                core_machine_port_write(port, 0x03f2u, 0x0cu);
                core_machine_fdc_change_require(&failed, &first_failure, 14,
                    (core_machine_port_read(port, 8u) & 0x40u) != 0u ||
                    core_machine_port_read(port, 0x03f4u) != TEST_FDC_MSR_CB);
                test_fdc_advance(fdc);
                for (lib_u8 index = 0u; index < 7u; ++index) {
                    const lib_u8 value = (lib_u8)core_machine_port_read(port, 0x03f5u);
                    core_machine_fdc_change_require(&failed, &first_failure, 14,
                        (index == 0u && value != 0x40u) ||
                        (index == 1u && value != 0x04u));
                }
                core_machine_fdc_change_require(&failed, &first_failure, 14,
                    core_machine_port_read(port, 0x03f4u) != TEST_FDC_MSR_RQM ||
                    fdc->connect.irq_source.asserted);
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                failed |= !core_machine_fdc_change_drain_reset(fdc, port);
                core_machine_fdc_change_command(fdc, port, read_0, sizeof(read_0));
                core_machine_fdc_change_require(&failed, &first_failure, 15,
                    (core_machine_port_read(port, 8u) & 0x40u) == 0u);
                core_machine_port_write(port, 0x03f2u, 0x00u);
                core_machine_fdc_change_require(&failed, &first_failure, 16,
                    (core_machine_port_read(port, 8u) & 0x40u) != 0u ||
                    !test_fdc_interrupt_matches(fdc, LIB_FALSE) ||
                    core_machine_port_read(port, 0x03f4u) != TEST_FDC_MSR_RQM);
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                failed |= !core_machine_fdc_change_drain_reset(fdc, port);
                core_machine_fdc_change_command(fdc, port, recalibrate_0,
                    sizeof(recalibrate_0));
                failed |= !test_fdc_finish_seeks(fdc);
                core_machine_fdc_change_require(&failed, &first_failure, 17,
                    !test_fdc_interrupt_matches(fdc, LIB_TRUE));
                core_machine_port_write(port, 0x03f2u, 0x00u);
                core_machine_fdc_change_require(&failed, &first_failure, 17,
                    !test_fdc_interrupt_matches(fdc, LIB_FALSE));
            }
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    if (failed) {
        fprintf(stderr, "M5:T380:S2:FDC-MEDIA-CHANGE:FAIL:step=%d\n",
            first_failure);
        return 1;
    }
    puts("M5:T291:S1:FDC:PORT:OK");
    return 0;
}
