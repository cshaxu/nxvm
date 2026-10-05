#include "lib/types/types_interface.h"
#include "port_assembly_board_fixture.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "x86/chips/fdc8272/fdc8272_interface.h"
#include "x86/chips/hdc/hdc_interface.h"

static lib_bool fail_fdc_create;
static lib_status port_assembly_create_fdc(const x86_fdc_connection *connection,
    const x86_fdc_timing *timing, x86_fdc **out_fdc)
{
    if (fail_fdc_create) {
        *out_fdc = LIB_NULL;
        return LIB_STATUS_NO_MEMORY;
    }
    return x86_fdc_create(connection, timing, out_fdc);
}

/* Inject a failed dependency at the adapter's existing create boundary. */
#define x86_fdc_create port_assembly_create_fdc
#include "ibmpc/board-common/fdc.c"
#undef x86_fdc_create

static lib_bool fail_hdc_create;
static lib_status port_assembly_create_hdc(const x86_hdc_config *config,
    const x86_hdc_connection *connection, x86_hdc **out_hdc)
{
    if (fail_hdc_create) {
        *out_hdc = LIB_NULL;
        return LIB_STATUS_NO_MEMORY;
    }
    return x86_hdc_create(config, connection, out_hdc);
}

#define x86_hdc_create port_assembly_create_hdc
#include "ibmpc/board-common/hdc.c"
#undef x86_hdc_create

test_port_assembly_board_observation test_port_assembly_board_capture(
    const core_machine_board_state *board)
{
    const core_machine_fdc fdc_zero = {0};
    const core_machine_fdc_topology fdc_topology_zero = {0};
    const core_machine_hdc hdc_zero = {0};
    const core_machine_hdc_topology hdc_topology_zero = {0};
    const core_machine_rtc_cmos_config rtc_zero = {0};
    const test_port_assembly_board_observation observed = {
        .fdc_configured = board->fdc_configured != 0u,
        .fdc_zero = lib_memory_compare(board->fdc, &fdc_zero,
            sizeof(fdc_zero)) == 0,
        .fdc_topology_zero = lib_memory_compare(&board->fdc_topology,
            &fdc_topology_zero, sizeof(fdc_topology_zero)) == 0,
        .rtc_configured = board->rtc_cmos_configured != 0u,
        .rtc_present = board->shared_rtc != LIB_NULL,
        .rtc_config_zero = lib_memory_compare(&board->rtc_cmos_config,
            &rtc_zero, sizeof(rtc_zero)) == 0,
        .hdc_configured = board->hdc_configured != 0u,
        .hdc_zero = lib_memory_compare(board->hdc, &hdc_zero,
            sizeof(hdc_zero)) == 0,
        .hdc_topology_zero = lib_memory_compare(&board->hdc_topology,
            &hdc_topology_zero, sizeof(hdc_topology_zero)) == 0,
        .hdc_dma_unbound = board->hdc_dma_request.core_token == 0u,
        .parity_bound = board->planar_parity != LIB_NULL,
        .profile_bound = board->profile_binding.context != LIB_NULL
    };
    return observed;
}

lib_status test_port_assembly_fdc_setup(core_machine_board_state *board,
    core_machine_dma_bus **out_dma, core_machine_dma_request_binding *out_request)
{
    const core_machine_dma_wiring wiring = {.fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL};
    const lib_status status = core_machine_configure_dma(board, &wiring, out_request);
    *out_dma = board->shared_dma;
    return status;
}

lib_status test_port_assembly_fdc_try(core_machine_board_state *board,
    const core_machine_media_registry *media,
    const core_machine_dma_request_binding *request, lib_bool fail_chip)
{
    const core_machine_fdc_topology topology = {
        .media_registry = media,
        .drives = {{1u, CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID}},
        .dma_request = *request,
        .config = {.dor_port = 0x03f2u, .status_port = 0x03f4u,
            .data_port = 0x03f5u, .direction_port = 0x03f7u,
            .control_port = 0x03f7u, .diagnostic_port = 0x03f0u,
            .irq = 6u, .dma_channel = 2u}
    };
    fail_fdc_create = fail_chip;
    const lib_status status = core_machine_configure_fdc(board, &topology);
    fail_fdc_create = LIB_FALSE;
    return status;
}

lib_status test_port_assembly_hdc_setup(core_machine_board_state *board,
    const core_machine_media_registry *media, core_machine_hdc_protocol protocol,
    core_machine_dma_bus **out_dma)
{
    const lib_bool xebec = protocol == CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT;
    const lib_bool compaq = protocol == CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB;
    *out_dma = LIB_NULL;
    if (xebec || compaq) {
        const core_machine_dma_wiring wiring = {.fdc_channel = 2u,
            .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
            .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL};
        core_machine_fdc_topology fdc = {.media_registry = media,
            .drives = {{1u, 0u, 0u, 0u}},
            .config = {.dor_port = 0x03f2u, .status_port = 0x03f4u,
                .data_port = 0x03f5u, .direction_port = 0x03f7u,
                .control_port = 0x03f7u, .irq = 6u, .dma_channel = 2u}};
        const lib_status status = core_machine_configure_dma(board, &wiring,
            &fdc.dma_request);
        *out_dma = board->shared_dma;
        if (status != LIB_STATUS_OK) return status;
        if (compaq) return core_machine_configure_fdc(board, &fdc);
    }
    return LIB_STATUS_OK;
}

lib_status test_port_assembly_hdc_try(core_machine_board_state *board,
    const core_machine_media_registry *media, core_machine_hdc_protocol protocol,
    lib_bool fail_chip)
{
    core_machine_hdc_topology topology = {
        .media_registry = media, .media_id = 1u,
        .config = {.protocol = protocol, .irq = 14u,
            .bus.task_file = {
                .data_port = 0x01f0u, .error_features_port = 0x01f1u,
                .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
                .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
                .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
                .alternate_status_device_control_port = 0x03f6u,
                .lba28_supported = protocol == CORE_MACHINE_HDC_PROTOCOL_ATA_PIO,
                .clock_ticks_per_second = 8000000u}}
    };
    if (protocol == CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB)
        topology.config.bus.task_file.drive_address_port = 0x03f7u;
    if (protocol == CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT) {
        topology.config.irq = 5u;
        topology.config.bus.xebec = (core_machine_hdc_xebec_config){
            .data_port = 0x0320u, .hardware_status_reset_port = 0x0321u,
            .jumpers_select_port = 0x0322u, .dma_irq_mask_port = 0x0323u,
            .dma_channel = 3u, .drive_type = CORE_MACHINE_XEBEC_DRIVE_TYPE_2,
            .expected_media_geometry = {CORE_MACHINE_XEBEC_TYPE_2_LOGICAL_SECTOR_COUNT,
                CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR, CORE_MACHINE_XEBEC_TYPE_2_CYLINDERS,
                CORE_MACHINE_XEBEC_TYPE_2_HEADS, CORE_MACHINE_XEBEC_TYPE_2_SECTORS_PER_TRACK}};
    }
    fail_hdc_create = fail_chip;
    const lib_status status = core_machine_configure_hdc(board, &topology);
    fail_hdc_create = LIB_FALSE;
    return status;
}

lib_status test_port_assembly_hdc_address_read(core_machine_board_state *board,
    lib_u32 *out_value)
{
    return core_machine_hdc_port_provider()->read(board->hdc, 0x03f7u, 0u, out_value);
}

lib_i32 test_port_assembly_refresh_count(core_machine_board_state *board)
{
    x86_pit *pit = board->shared_pit;
    lib_u8 low = 0u, high = 0u;

    /* The reload becomes observable on the next input-clock cycle. */
    x86_pit_advance(pit, 1u);
    return x86_pit_write_register(pit, 3u, 0x40u) != LIB_STATUS_OK ||
        x86_pit_read_counter(pit, 1u, &low) != LIB_STATUS_OK ||
        x86_pit_read_counter(pit, 1u, &high) != LIB_STATUS_OK ||
        low != 18u || high != 0u;
}
