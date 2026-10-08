#include "lib/types/file.h"
#include "fdc_fixture.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "lib/types/types_interface.h"
#include "ibmpc/board-common/dma_bus_interface.h"
#include "ibmpc/board-common/fdc.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "ibmpc/board-common/media_interface.h"

typedef struct core_machine_fdc_topology_media {
    lib_u8 byte;
    lib_u32 read_count;
} core_machine_fdc_topology_media;

static core_machine_media_result core_machine_fdc_topology_query(void *context,
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

static core_machine_media_result core_machine_fdc_topology_read(void *context,
    lib_u64 offset, void *buffer, lib_u32 byte_count)
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

static void core_machine_fdc_topology_command(core_machine_fdc *fdc, core_machine *port,
    const lib_u8 *bytes, lib_size count)
{
    lib_size index;

    for (index = 0u; index < count; ++index) {
        test_fdc_port_write(port, 0x03f5u, bytes[index]);
    }
    test_fdc_advance(fdc);
}

static lib_i32 core_machine_fdc_topology_result(core_machine_fdc *fdc, core_machine *port,
    lib_u8 *result, lib_size count)
{
    lib_size index;

    test_fdc_advance(fdc);
    for (index = 0u; index < count; ++index) {
        result[index] = (lib_u8)test_fdc_port_read(port, 0x03f5u);
    }
    return (test_fdc_port_read(port, 0x03f4u) & (TEST_FDC_MSR_CB | TEST_FDC_MSR_DIO)) == 0u;
}

static lib_i32 core_machine_fdc_topology_read_sector(core_machine_fdc *fdc, core_machine *port,
    lib_u8 unit, lib_u8 expected, lib_u8 *result)
{
    const lib_u8 command[] = {0xe6u, unit, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu};
    lib_u32 index;

    core_machine_fdc_topology_command(fdc, port, command, sizeof(command));
    if (test_fdc_port_read(port, 0x03f5u) != expected) return 0;
    for (index = 1u; index < 512u; ++index) {
        if (!test_fdc_advance_ticks(fdc, 128u)) return 0;
        (void)test_fdc_port_read(port, 0x03f5u);
    }
    return core_machine_fdc_topology_result(fdc, port, result, 7u) &&
        result[0] == unit && result[1] == 0u;
}

int main(void)
{
    static const lib_u8 specify_non_dma[] = {0x03u, 0xdfu, 0x03u};
    lib_u8 sense_drive[] = {0x04u, 0u};
    static const lib_u8 read_absent[] = {0xe6u, 2u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu};
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
        .clock_ticks_per_second = 8000000u
    };
    const core_machine_fdc_drive_bindings drives = {
        {11u, 12u, CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}, 0x03u, 0x03u,
        {0u, 0u, 0u, 0u}, 0u, {0}
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
    core_machine_board_state *board = LIB_NULL;
    core_machine_fdc *fdc = LIB_NULL;
    lib_u8 diagnostic_phase = 0u;
    lib_u8 reset_drive;
    core_machine *port = LIB_NULL;
    lib_u8 result[7] = {0};
    lib_i32 failed = 0;

    if (core_machine_media_registry_create(&media) != LIB_STATUS_OK ||
        core_machine_create(&config, &machine, &board) != LIB_STATUS_OK) failed |= 0x01;
    if (!failed) {
        fdc = board->fdc;
        port = machine;
        if (fdc == LIB_NULL || port == LIB_NULL ||
            core_machine_media_registry_bind(media, 11u, &drive0,
                &core_machine_fdc_topology_provider) != LIB_STATUS_OK ||
            core_machine_media_registry_bind(media, 12u, &drive1,
                &core_machine_fdc_topology_provider) != LIB_STATUS_OK ||
            core_machine_media_registry_freeze(media) != LIB_STATUS_OK ||
            core_machine_media_registry_bind(media, 13u, &drive0,
                &core_machine_fdc_topology_provider) != LIB_STATUS_INVALID_STATE ||
            core_machine_configure_dma(board, &dma_wiring, &dma_request) !=
                LIB_STATUS_OK) {
            failed |= 0x02;
        } else {
            topology.media_registry = media;
            topology.drives = drives;
            topology.dma_request = dma_request;
            topology.config = fdc_config;
            if (core_machine_configure_fdc(board, &topology) != LIB_STATUS_OK ||
                core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                core_machine_reset(machine) != LIB_STATUS_OK) {
                failed |= 0x04;
            } else {
                test_fdc_port_write(port, 0x03f2u, 0x1cu);
                failed |= core_machine_pic_irq_source_is_asserted(fdc->connect.irq_source) ? 0x08 : 0;
                failed |= !test_fdc_advance_due(fdc) ? 0x08 : 0;
                failed |= !core_machine_pic_irq_source_is_asserted(fdc->connect.irq_source) ? 0x08 : 0;
                for (reset_drive = 0u; reset_drive < CORE_MACHINE_FDC_DRIVE_COUNT;
                    ++reset_drive) {
                    core_machine_fdc_topology_command(fdc, port,
                        (const lib_u8[]){0x08u}, 1u);
                    failed |= (!core_machine_fdc_topology_result(fdc, port, result, 2u) ||
                        result[0] != (TEST_FDC_ST0_READY_CHANGE | reset_drive) ||
                        result[1] != 0u || core_machine_pic_irq_source_is_asserted(fdc->connect.irq_source)) ? 0x08 : 0;
                }
                test_fdc_port_write(port, 0x03f2u, 0x1cu);
                core_machine_fdc_topology_command(fdc, port,
                    (const lib_u8[]){0x08u}, 1u);
                failed |= (!core_machine_fdc_topology_result(fdc, port, result, 1u) ||
                    result[0] != 0x80u ||
                    core_machine_pic_irq_source_is_asserted(fdc->connect.irq_source)) ? 0x08 : 0;
                core_machine_fdc_topology_command(fdc, port, specify_non_dma,
                    sizeof(specify_non_dma));
                core_machine_fdc_topology_command(fdc, port, sense_drive,
                    sizeof(sense_drive));
                failed |= (!core_machine_fdc_topology_result(fdc, port, result, 1u) ||
                    result[0] != 0x38u ||
                    !core_machine_fdc_topology_read_sector(fdc, port, 0u, 0xa1u, result) ||
                    drive0.read_count != 512u || drive1.read_count != 0u) ? 0x10 : 0;

                test_fdc_port_write(port, 0x03f2u, 0x2du);
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
                    result[0] != TEST_FDC_ST0_ABNORMAL || result[1] != 0x04u ||
                    drive0.read_count != 512u || drive1.read_count != 512u) ? 0x40 : 0;

                test_fdc_port_write(port, 0x03f2u, 0x4eu);
                sense_drive[1] = 2u;
                core_machine_fdc_topology_command(fdc, port, sense_drive,
                    sizeof(sense_drive));
                failed |= (!core_machine_fdc_topology_result(fdc, port, result, 1u) ||
                    result[0] != 0x22u) ? 0x80 : 0;
                core_machine_fdc_topology_command(fdc, port,
                    (const lib_u8[]){0x07u, 2u}, 2u);
                for (lib_u8 step = 0u; step < 77u &&
                        (test_fdc_port_read(port, 0x03f4u) & 0x04u) != 0u; ++step)
                    failed |= !test_fdc_advance_due(fdc) ? 0x100 : 0;
                core_machine_fdc_topology_command(fdc, port,
                    (const lib_u8[]){0x08u}, 1u);
                failed |= (!core_machine_fdc_topology_result(fdc, port, result, 2u) ||
                    result[0] != (TEST_FDC_ST0_ABNORMAL |
                        TEST_FDC_ST0_SEEK_END | TEST_FDC_ST0_EQUIPMENT_CHECK | 2u) ||
                    result[1] != 0u) ? 0x100 : 0;
                core_machine_fdc_topology_command(fdc, port,
                    (const lib_u8[]){0x0fu, 2u, 1u}, 3u);
                failed |= !test_fdc_advance_due(fdc) ? 0x200 : 0;
                core_machine_fdc_topology_command(fdc, port,
                    (const lib_u8[]){0x08u}, 1u);
                failed |= (!core_machine_fdc_topology_result(fdc, port, result, 2u) ||
                    result[0] != (TEST_FDC_ST0_SEEK_END | 2u) ||
                    result[1] != 1u) ? 0x200 : 0;
                core_machine_fdc_topology_command(fdc, port, read_absent, sizeof(read_absent));
                failed |= (!core_machine_fdc_topology_result(fdc, port, result, 7u) ||
                    result[0] != 0x42u || result[1] != 0x04u ||
                    drive0.read_count != 512u || drive1.read_count != 512u) ? 0x400 : 0;
            }
        }
    }
    if (fdc != LIB_NULL) {
        x86_fdc_observation observation;
        if (x86_fdc_capture(fdc->chip, &observation) == LIB_STATUS_OK)
            diagnostic_phase = observation.phase;
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    if (failed) {
        lib_c_fprintf(lib_c_stderr, "FDC-TOPOLOGY:FAIL:%x:reads=%u,%u:phase=%u\n",
            failed, drive0.read_count, drive1.read_count,
            diagnostic_phase);
        return 1;
    }
    lib_c_printf("%s\n", "FDC:PORT:OK");
    return 0;
}
