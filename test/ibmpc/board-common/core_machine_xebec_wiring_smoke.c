#include "lib/types/file.h"
#include "pic_fixture.h"
#include "hdc_fixture.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "dma_fixture.h"
#include "lib/types/types_interface.h"
#include "x86/core/machine_interface.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "ibmpc/board-common/media_interface.h"

typedef struct xebec_media {
    lib_u8 bytes[2u * CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR];
} xebec_media;

static core_machine_media_result xebec_media_query(void *opaque,
    core_machine_media_info *out_info)
{
    (void)opaque;
    if (out_info == LIB_NULL) return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    *out_info = (core_machine_media_info) {
        .present = LIB_TRUE,
        .capabilities = CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN,
        .geometry = {
            CORE_MACHINE_XEBEC_TYPE_2_LOGICAL_SECTOR_COUNT,
            CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR,
            CORE_MACHINE_XEBEC_TYPE_2_CYLINDERS,
            CORE_MACHINE_XEBEC_TYPE_2_HEADS,
            CORE_MACHINE_XEBEC_TYPE_2_SECTORS_PER_TRACK}
    };
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result xebec_media_read(void *opaque,
    lib_u64 offset, void *buffer, lib_u32 count)
{
    xebec_media *media = opaque;

    if (media == LIB_NULL || buffer == LIB_NULL || offset >
        sizeof(media->bytes) - CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR ||
        count != CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    lib_memory_copy(buffer, media->bytes + offset, count);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result xebec_media_write(void *opaque,
    lib_u64 offset, const void *buffer, lib_u32 count)
{
    xebec_media *media = opaque;

    if (media == LIB_NULL || buffer == LIB_NULL || offset >
        sizeof(media->bytes) - CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR ||
        count != CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    lib_memory_copy(media->bytes + offset, buffer, count);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider xebec_media_provider = {
    xebec_media_query, xebec_media_read, xebec_media_write,
    LIB_NULL, LIB_NULL, LIB_NULL, LIB_NULL
};

static lib_bool xebec_routes_match(core_machine *machine)
{
    const struct {
        lib_u16 port;
        lib_bool write;
        lib_bool present;
    } routes[] = {
        {0x320u, LIB_FALSE, LIB_TRUE}, {0x320u, LIB_TRUE, LIB_TRUE},
        {0x321u, LIB_FALSE, LIB_TRUE}, {0x321u, LIB_TRUE, LIB_TRUE},
        {0x322u, LIB_FALSE, LIB_TRUE}, {0x322u, LIB_TRUE, LIB_TRUE},
        {0x323u, LIB_FALSE, LIB_FALSE}, {0x323u, LIB_TRUE, LIB_TRUE},
        {0x1f0u, LIB_FALSE, LIB_FALSE}, {0x1f7u, LIB_TRUE, LIB_FALSE}
    };
    const core_machine_port_provider *provider = core_machine_hdc_port_provider();

    /* A conflicting registration proves the route exists even when the
     * device rejects that read. Successful probes are removed before freeze. */
    for (lib_size index = 0u; index < sizeof(routes) / sizeof(routes[0]); ++index) {
        const core_machine_port_route route = {
            .address = routes[index].port,
            .read = routes[index].write ? LIB_NULL : provider->read,
            .write = routes[index].write ? provider->write : LIB_NULL,
            .owner = (void *)routes
        };
        lib_status status = core_machine_install_port_routes(machine, &route, 1u);
        if (status == LIB_STATUS_OK &&
            core_machine_remove_port_routes(machine, routes) != LIB_STATUS_OK) return LIB_FALSE;
        if (status != (routes[index].present ? LIB_STATUS_INVALID_STATE : LIB_STATUS_OK))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static void xebec_configure_dma3(core_machine *port, lib_u16 address,
    lib_u16 count, lib_u8 mode)
{
    test_hdc_port_write(port, 0x000cu, 0u);
    test_hdc_port_write(port, 0x0006u, address & 0xffu);
    test_hdc_port_write(port, 0x0006u, address >> 8u);
    test_hdc_port_write(port, 0x0007u, count & 0xffu);
    test_hdc_port_write(port, 0x0007u, count >> 8u);
    test_hdc_port_write(port, 0x0082u, 0u);
    test_hdc_port_write(port, 0x000bu, mode);
    test_hdc_port_write(port, 0x000au, 0x03u);
}

lib_i32 main(void)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8088,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_dma_wiring dma = {
        .fdc_channel = CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND,
        .controller_count = 1u,
        .cascade_channel = 0u
    };
    const core_machine_hdc_topology hdc = {
        .media_registry = LIB_NULL,
        .media_id = 1u,
        .slave_media_id = CORE_MACHINE_MEDIA_ID_INVALID,
        .config = {
            .protocol = CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT,
            .irq = 5u,
            .service = {250u, 0u},
            .bus.xebec = {
                .data_port = 0x0320u,
                .hardware_status_reset_port = 0x0321u,
                .jumpers_select_port = 0x0322u,
                .dma_irq_mask_port = 0x0323u,
                .dma_channel = 3u,
                .drive_type = CORE_MACHINE_XEBEC_DRIVE_TYPE_2,
                .expected_media_geometry = {
                    CORE_MACHINE_XEBEC_TYPE_2_LOGICAL_SECTOR_COUNT,
                    CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR,
                    CORE_MACHINE_XEBEC_TYPE_2_CYLINDERS,
                    CORE_MACHINE_XEBEC_TYPE_2_HEADS,
                    CORE_MACHINE_XEBEC_TYPE_2_SECTORS_PER_TRACK}
            }
        }
    };
    core_machine_media_registry *registry = LIB_NULL;
    xebec_media media = {{0}};
    core_machine_dma_request_binding fdc_binding = {0};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_hdc_topology topology = hdc;
    const lib_u8 dcb[] = {0x00u, 0x20u, 0x01u, 0x23u, 0u, 0u};
    const lib_u8 initialize_dcb[] = {0x0cu, 0u, 0u, 0u, 0u, 0u};
    const lib_u8 response[] = {0x22u};
    const lib_u8 sense_dcb[] = {0x03u, 0x20u, 0u, 0u, 0u, 0u};
    const lib_u8 sense[] = {0x04u, 0u, 0u, 0u};
    const lib_u8 invalid_dcb[] = {0x02u, 0u, 0u, 0u, 0u, 0u};
    const lib_u8 read_dcb[] = {0x08u, 0u, 0u, 0u, 2u, 0u};
    const lib_u8 write_dcb[] = {0x0au, 0u, 0u, 0u, 1u, 0u};
    const core_machine_dma_channel_provider *dma_provider;
    lib_u8 dma_bytes[2u * CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR];
    lib_u64 due_tick = 0u;
    lib_size index;
    lib_i32 failed = 0;

    if (core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_media_registry_create(&registry) != LIB_STATUS_OK) failed |= 0x01;
    if (!failed) {
        for (index = 0u; index < sizeof(media.bytes); ++index)
            media.bytes[index] = (lib_u8)(index < 512u ? index : 0xa5u);
        if (core_machine_media_registry_bind(registry, 1u, &media,
                &xebec_media_provider) != LIB_STATUS_OK ||
            core_machine_media_registry_freeze(registry) != LIB_STATUS_OK) failed |= 0x02;
        if (failed) goto cleanup;
        topology.media_registry = registry;
        if (core_machine_configure_dma(board, &dma, &fdc_binding) != LIB_STATUS_OK ||
            core_machine_configure_hdc(board, &topology) != LIB_STATUS_OK) {
            failed |= 0x04;
        } else if (!xebec_routes_match(machine) ||
            core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK) {
            failed |= 0x08;
        } else if (
            board->hdc->connect.config.protocol != CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT ||
            !test_pic_source_route(board->shared_pic_master, board->shared_pic_slave,
                board->hdc->connect.irq_source, 5u)) {
            failed |= 0x10;
        } else if (
            board->hdc_dma_request.core_token == 0u ||
            board->hdc_dma_request.channel != 3u) {
            failed |= 0x20;
        } else {
            test_hdc_port_write(machine, 0x0320u, dcb[0]);
            if (hdc_observe(board->hdc).xebec_dcb_count != 0u) failed |= 0x40;
            if (failed) goto cleanup;
            test_hdc_port_write(machine, 0x0322u, 0u);
            for (index = 0u; index < sizeof(dcb); ++index)
                test_hdc_port_write(machine, 0x0320u, dcb[index]);
            if (hdc_observe(board->hdc).xebec_phase != X86_XEBEC_PHASE_PENDING_COMMAND ||
                core_machine_hdc_next_due_tick(board->hdc, &due_tick) != LIB_STATUS_OK ||
                due_tick != hdc_observe(board->hdc).elapsed_ticks + 250u ||
                core_machine_hdc_irq_pending(board->hdc)) failed |= 0x80;
            if (failed) goto cleanup;
            hdc_service(board->hdc);
            for (index = 0u; index < sizeof(response); ++index) {
                if (test_hdc_port_read(machine, 0x0320u) != response[index]) {
                    failed |= 0x80;
                    break;
                }
            }
            if (!failed) {
                dma_provider = core_machine_hdc_dma_provider();
                xebec_configure_dma3(machine, 0x2200u, 1023u, 0x87u);
                test_hdc_port_write(machine, 0x0323u, 0x03u);
                test_hdc_port_write(machine, 0x0322u, 0u);
                for (index = 0u; index < sizeof(read_dcb); ++index)
                    test_hdc_port_write(machine, 0x0320u, read_dcb[index]);
                hdc_service(board->hdc);
                if (dma_provider == LIB_NULL || dma_provider->read_device == LIB_NULL ||
                    hdc_observe(board->hdc).xebec_phase != X86_XEBEC_PHASE_DMA_READ ||
                    (test_hdc_port_read(machine, 8u) & 0x80u) == 0u)
                    failed |= 0x100;
                if (failed) goto cleanup;
                test_dma_transfers(board->shared_dma,
                    machine, machine, sizeof(dma_bytes), 1u);
                if (!failed && (core_machine_memory_read(machine,
                    0x2200u, dma_bytes, sizeof(dma_bytes)) !=
                    LIB_STATUS_OK || dma_bytes[0] != 0u || dma_bytes[511] != 0xffu ||
                    dma_bytes[512] != 0xa5u || dma_bytes[1023] != 0xa5u))
                    failed |= 0x200;
                if (!failed && (hdc_observe(board->hdc).xebec_phase != X86_XEBEC_PHASE_RESPONSE ||
                    (test_hdc_port_read(machine, 8u) & 0x80u) != 0u ||
                    test_hdc_port_read(machine, 0x0320u) != 0u)) failed |= 0x400;
            }
            if (!failed) {
                test_hdc_port_write(machine, 0x0322u, 0u);
                for (index = 0u; index < sizeof(read_dcb); ++index)
                    test_hdc_port_write(machine, 0x0320u, read_dcb[index]);
                hdc_service(board->hdc);
                dma_provider->terminal_count(board->hdc, &(t_latch){0});
                if (hdc_observe(board->hdc).xebec_phase != X86_XEBEC_PHASE_RESPONSE ||
                    (test_hdc_port_read(machine, 8u) & 0x80u) != 0u ||
                    test_hdc_port_read(machine, 0x0320u) != 0x02u) failed |= 0x800;
            }
            if (!failed) {
                dma_provider = core_machine_hdc_dma_provider();
                for (index = 0u; index < sizeof(dma_bytes); ++index)
                    dma_bytes[index] = (lib_u8)(0xffu - index);
                if (core_machine_memory_write(machine,
                        0x2400u, dma_bytes,
                        sizeof(dma_bytes)) != LIB_STATUS_OK) failed |= 0x1000;
                if (failed) goto cleanup;
                xebec_configure_dma3(machine, 0x2400u, 511u, 0x8bu);
                test_hdc_port_write(machine, 0x0322u, 0u);
                for (index = 0u; index < sizeof(write_dcb); ++index)
                    test_hdc_port_write(machine, 0x0320u, write_dcb[index]);
                hdc_service(board->hdc);
                if (dma_provider == LIB_NULL || dma_provider->write_device == LIB_NULL ||
                    hdc_observe(board->hdc).xebec_phase != X86_XEBEC_PHASE_DMA_WRITE ||
                    (test_hdc_port_read(machine, 8u) & 0x80u) == 0u)
                    failed |= 0x2000;
                if (failed) goto cleanup;
                test_dma_transfers(board->shared_dma,
                    machine, machine, sizeof(dma_bytes), 1u);
                if (!failed && (hdc_observe(board->hdc).xebec_phase != X86_XEBEC_PHASE_RESPONSE ||
                    test_hdc_port_read(machine, 0x0320u) != 0u ||
                    (test_hdc_port_read(machine, 8u) & 0x80u) != 0u ||
                    media.bytes[0] != 0xffu || media.bytes[511] != 0u)) failed |= 0x4000;
            }
            if (!failed) {
                test_hdc_port_write(machine, 0x0322u, 0u);
                for (index = 0u; index < sizeof(sense_dcb); ++index)
                    test_hdc_port_write(machine, 0x0320u, sense_dcb[index]);
                hdc_service(board->hdc);
                for (index = 0u; index < sizeof(sense); ++index) {
                    if (test_hdc_port_read(machine, 0x0320u) != sense[index]) {
                        failed |= 0x8000;
                        break;
                    }
                }
            }
            if (!failed) {
                test_hdc_port_write(machine, 0x0322u, 0u);
                for (index = 0u; index < sizeof(invalid_dcb); ++index)
                    test_hdc_port_write(machine, 0x0320u, invalid_dcb[index]);
                hdc_service(board->hdc);
                if (test_hdc_port_read(machine, 0x0320u) != 0x02u)
                    failed |= 0x10000;
            }
            if (!failed) {
                test_hdc_port_write(machine, 0x0323u, 0x02u);
                test_hdc_port_write(machine, 0x0322u, 0u);
                for (index = 0u; index < sizeof(dcb); ++index)
                    test_hdc_port_write(machine, 0x0320u, dcb[index]);
                hdc_service(board->hdc);
                if (!core_machine_hdc_irq_pending(board->hdc) ||
                    test_hdc_port_read(machine, 0x0320u) != response[0] ||
                    core_machine_hdc_irq_pending(board->hdc)) failed |= 0x20000;
            }
            if (!failed) {
                test_hdc_port_write(machine, 0x0323u, 0u);
                test_hdc_port_write(machine, 0x0322u, 0u);
                for (index = 0u; index < sizeof(read_dcb); ++index)
                    test_hdc_port_write(machine, 0x0320u, read_dcb[index]);
                hdc_service(board->hdc);
                if (hdc_observe(board->hdc).xebec_phase != X86_XEBEC_PHASE_DMA_READ ||
                    (test_hdc_port_read(machine, 8u) & 0x80u) != 0u) {
                    failed |= 0x20000;
                }
                if (failed) goto cleanup;
                test_hdc_port_write(machine, 0x0323u, 0x01u);
                if ((test_hdc_port_read(machine, 8u) & 0x80u) == 0u) {
                    failed |= 0x20000;
                }
                if (failed) goto cleanup;
                dma_provider->terminal_count(board->hdc, &(t_latch){0});
                (void)test_hdc_port_read(machine, 0x0320u);
            }
            if (!failed) {
                test_hdc_port_write(machine, 0x0323u, 0x5au);
                if (hdc_observe(board->hdc).xebec_mask != 0x5au) failed |= 0x20000;
            }
            if (!failed) {
                test_hdc_port_write(machine, 0x0322u, 0u);
                for (index = 0u; index < sizeof(initialize_dcb); ++index)
                    test_hdc_port_write(machine, 0x0320u, initialize_dcb[index]);
                if (hdc_observe(board->hdc).xebec_phase != X86_XEBEC_PHASE_INITIALIZE) failed |= 0x40000;
                if (failed) goto cleanup;
                for (index = 0u; index < sizeof(hdc_observe(board->hdc).xebec_initialize); ++index)
                    test_hdc_port_write(machine, 0x0320u, 0u);
                hdc_service(board->hdc);
                if (hdc_observe(board->hdc).xebec_phase != X86_XEBEC_PHASE_RESPONSE) failed |= 0x80000;
            }
            if (!failed) {
                test_hdc_port_write(machine, 0x0321u, 0u);
                if (hdc_observe(board->hdc).xebec_dcb_count != 0u ||
                    hdc_observe(board->hdc).xebec_mask != 0u ||
                    hdc_observe(board->hdc).xebec_phase != X86_XEBEC_PHASE_IDLE) failed |= 0x100000;
            }
        }
    }
cleanup:
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(registry);
    if (failed) {
        lib_c_fprintf(lib_c_stderr, "XEBEC-STACK:FAIL bits=%x\n", failed);
        return 1;
    }
    lib_c_printf("%s\n", "XEBEC-STACK:OK");
    lib_c_printf("%s\n", "XEBEC-NO-ATA-ALIAS:OK");
    lib_c_printf("%s\n", "XEBEC-DMA-MEDIA:OK");
    lib_c_printf("%s\n", "XEBEC-DMA-RAM:OK");
    return 0;
}
