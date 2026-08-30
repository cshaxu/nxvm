#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/media_interface.h"
#include "core/machine/port.h"

typedef struct xebec_media {
    type_unsigned_8 bytes[2u * CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR];
} xebec_media;

static core_machine_media_result xebec_media_query(C_VOID *opaque,
    core_machine_media_info *out_info)
{
    (C_VOID)opaque;
    if (out_info == STD_NULL) return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    *out_info = (core_machine_media_info) {
        .present = TYPE_TRUE,
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

static core_machine_media_result xebec_media_read(C_VOID *opaque,
    type_unsigned_64 offset, C_VOID *buffer, type_unsigned_32 count)
{
    xebec_media *media = opaque;

    if (media == STD_NULL || buffer == STD_NULL || offset >
        sizeof(media->bytes) - CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR ||
        count != CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    STD_MEMCPY(buffer, media->bytes + offset, count);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result xebec_media_write(C_VOID *opaque,
    type_unsigned_64 offset, const C_VOID *buffer, type_unsigned_32 count)
{
    xebec_media *media = opaque;

    if (media == STD_NULL || buffer == STD_NULL || offset >
        sizeof(media->bytes) - CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR ||
        count != CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    STD_MEMCPY(media->bytes + offset, buffer, count);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider xebec_media_provider = {
    xebec_media_query, xebec_media_read, xebec_media_write,
    STD_NULL, STD_NULL, STD_NULL, STD_NULL
};

static C_VOID xebec_configure_dma3(t_port *port, type_unsigned_16 address,
    type_unsigned_16 count, type_unsigned_8 mode)
{
    core_machine_port_write(port, 0x000cu, 0u);
    core_machine_port_write(port, 0x0006u, address & 0xffu);
    core_machine_port_write(port, 0x0006u, address >> 8u);
    core_machine_port_write(port, 0x0007u, count & 0xffu);
    core_machine_port_write(port, 0x0007u, count >> 8u);
    core_machine_port_write(port, 0x0082u, 0u);
    core_machine_port_write(port, 0x000bu, mode);
    core_machine_port_write(port, 0x000au, 0x03u);
}

C_INT main(C_VOID)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8088,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_dma_wiring dma = {
        .fdc_channel = CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND,
        .controller_count = 1u,
        .cascade_channel = 0u
    };
    const core_machine_hdc_topology hdc = {
        .media_registry = STD_NULL,
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
    core_machine_media_registry *registry = STD_NULL;
    xebec_media media = {{0}};
    core_machine_dma_request_binding fdc_binding = {0};
    core_machine *machine = STD_NULL;
    core_machine_hdc_topology topology = hdc;
    const type_unsigned_8 dcb[] = {0x00u, 0x20u, 0x01u, 0x23u, 0u, 0u};
    const type_unsigned_8 initialize_dcb[] = {0x0cu, 0u, 0u, 0u, 0u, 0u};
    const type_unsigned_8 response[] = {0x22u};
    const type_unsigned_8 sense_dcb[] = {0x03u, 0x20u, 0u, 0u, 0u, 0u};
    const type_unsigned_8 sense[] = {0x04u, 0u, 0u, 0u};
    const type_unsigned_8 invalid_dcb[] = {0x02u, 0u, 0u, 0u, 0u, 0u};
    const type_unsigned_8 read_dcb[] = {0x08u, 0u, 0u, 0u, 2u, 0u};
    const type_unsigned_8 write_dcb[] = {0x0au, 0u, 0u, 0u, 1u, 0u};
    const core_machine_dma_channel_provider *dma_provider;
    type_unsigned_8 dma_bytes[2u * CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR];
    type_unsigned_64 due_tick = 0u;
    STD_SIZE_T index;
    C_INT failed = 0;

    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_media_registry_create(&registry) != TYPE_STATUS_OK) failed |= 0x01;
    if (!failed) {
        for (index = 0u; index < sizeof(media.bytes); ++index)
            media.bytes[index] = (type_unsigned_8)(index < 512u ? index : 0xa5u);
        if (core_machine_media_registry_bind(registry, 1u, &media,
                &xebec_media_provider) != TYPE_STATUS_OK ||
            core_machine_media_registry_freeze(registry) != TYPE_STATUS_OK) failed |= 0x02;
        topology.media_registry = registry;
        if (!failed && (core_machine_configure_dma(machine, &dma, &fdc_binding) != TYPE_STATUS_OK ||
            core_machine_configure_hdc(machine, &topology) != TYPE_STATUS_OK)) {
            failed |= 0x04;
        } else if (!core_machine_port_has_read(&machine->executor_port, 0x0320u) ||
            !core_machine_port_has_write(&machine->executor_port, 0x0320u) ||
            !core_machine_port_has_read(&machine->executor_port, 0x0321u) ||
            !core_machine_port_has_write(&machine->executor_port, 0x0321u) ||
            !core_machine_port_has_read(&machine->executor_port, 0x0322u) ||
            !core_machine_port_has_write(&machine->executor_port, 0x0322u) ||
            core_machine_port_has_read(&machine->executor_port, 0x0323u) ||
            !core_machine_port_has_write(&machine->executor_port, 0x0323u) ||
            core_machine_port_has_read(&machine->executor_port, 0x01f0u) ||
            core_machine_port_has_write(&machine->executor_port, 0x01f7u)) {
            failed |= 0x08;
        } else if (
            machine->hdc.connect.config.protocol != CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT ||
            machine->hdc.connect.irq_source.irq != 5u) {
            failed |= 0x10;
        } else if (
            machine->hdc_dma_request.core_token == 0u ||
            machine->hdc_dma_request.channel != 3u) {
            failed |= 0x20;
        } else {
            core_machine_port_write(&machine->executor_port, 0x0320u, dcb[0]);
            if (machine->hdc.xebec.dcb_count != 0u) failed |= 0x40;
            core_machine_port_write(&machine->executor_port, 0x0322u, 0u);
            for (index = 0u; index < sizeof(dcb); ++index)
                core_machine_port_write(&machine->executor_port, 0x0320u, dcb[index]);
            if (machine->hdc.xebec.phase != CORE_MACHINE_XEBEC_PHASE_PENDING_COMMAND ||
                core_machine_hdc_next_due_tick(&machine->hdc, &due_tick) != TYPE_STATUS_OK ||
                due_tick != machine->hdc.data.elapsed_ticks + 250u ||
                core_machine_hdc_irq_pending(&machine->hdc)) failed |= 0x80;
            core_machine_hdc_advance(&machine->hdc);
            for (index = 0u; index < sizeof(response); ++index) {
                if (core_machine_port_read(&machine->executor_port, 0x0320u) != response[index]) {
                    failed |= 0x80;
                    break;
                }
            }
            if (!failed) {
                dma_provider = core_machine_hdc_dma_provider();
                xebec_configure_dma3(&machine->executor_port, 0x2200u, 1023u, 0x87u);
                core_machine_port_write(&machine->executor_port, 0x0323u, 0x03u);
                core_machine_port_write(&machine->executor_port, 0x0322u, 0u);
                for (index = 0u; index < sizeof(read_dcb); ++index)
                    core_machine_port_write(&machine->executor_port, 0x0320u, read_dcb[index]);
                core_machine_hdc_advance(&machine->hdc);
                if (dma_provider == STD_NULL || dma_provider->read_device == STD_NULL ||
                    machine->hdc.xebec.phase != CORE_MACHINE_XEBEC_PHASE_DMA_READ ||
                    (machine->shared_dma_primary.data.status & VDMA_STATUS_DRQ(3u)) == 0u)
                    failed |= 0x100;
                core_machine_dma_advance(&machine->shared_dma_latch,
                    &machine->shared_dma_primary, &machine->shared_dma_secondary,
                    &machine->executor_memory, sizeof(dma_bytes));
                if (!failed && (core_machine_memory_read_physical(&machine->executor_memory,
                    0x2200u, (type_virtual_address)dma_bytes, sizeof(dma_bytes)) !=
                    TYPE_STATUS_OK || dma_bytes[0] != 0u || dma_bytes[511] != 0xffu ||
                    dma_bytes[512] != 0xa5u || dma_bytes[1023] != 0xa5u))
                    failed |= 0x200;
                if (!failed && (machine->hdc.xebec.phase != CORE_MACHINE_XEBEC_PHASE_RESPONSE ||
                    (machine->shared_dma_primary.data.status & VDMA_STATUS_DRQ(3u)) != 0u ||
                    core_machine_port_read(&machine->executor_port, 0x0320u) != 0u)) failed |= 0x400;
            }
            if (!failed) {
                core_machine_port_write(&machine->executor_port, 0x0322u, 0u);
                for (index = 0u; index < sizeof(read_dcb); ++index)
                    core_machine_port_write(&machine->executor_port, 0x0320u, read_dcb[index]);
                core_machine_hdc_advance(&machine->hdc);
                dma_provider->terminal_count(&machine->hdc, &machine->shared_dma_latch);
                if (machine->hdc.xebec.phase != CORE_MACHINE_XEBEC_PHASE_RESPONSE ||
                    (machine->shared_dma_primary.data.status & VDMA_STATUS_DRQ(3u)) != 0u ||
                    core_machine_port_read(&machine->executor_port, 0x0320u) != 0x02u) failed |= 0x800;
            }
            if (!failed) {
                dma_provider = core_machine_hdc_dma_provider();
                for (index = 0u; index < sizeof(dma_bytes); ++index)
                    dma_bytes[index] = (type_unsigned_8)(0xffu - index);
                if (core_machine_memory_write_physical(&machine->executor_memory,
                        0x2400u, (type_virtual_address)dma_bytes,
                        sizeof(dma_bytes)) != TYPE_STATUS_OK) failed |= 0x1000;
                xebec_configure_dma3(&machine->executor_port, 0x2400u, 511u, 0x8bu);
                core_machine_port_write(&machine->executor_port, 0x0322u, 0u);
                for (index = 0u; index < sizeof(write_dcb); ++index)
                    core_machine_port_write(&machine->executor_port, 0x0320u, write_dcb[index]);
                core_machine_hdc_advance(&machine->hdc);
                if (dma_provider == STD_NULL || dma_provider->write_device == STD_NULL ||
                    machine->hdc.xebec.phase != CORE_MACHINE_XEBEC_PHASE_DMA_WRITE ||
                    (machine->shared_dma_primary.data.status & VDMA_STATUS_DRQ(3u)) == 0u)
                    failed |= 0x2000;
                core_machine_dma_advance(&machine->shared_dma_latch,
                    &machine->shared_dma_primary, &machine->shared_dma_secondary,
                    &machine->executor_memory, sizeof(dma_bytes));
                if (!failed && (machine->hdc.xebec.phase != CORE_MACHINE_XEBEC_PHASE_RESPONSE ||
                    core_machine_port_read(&machine->executor_port, 0x0320u) != 0u ||
                    (machine->shared_dma_primary.data.status & VDMA_STATUS_DRQ(3u)) != 0u ||
                    media.bytes[0] != 0xffu || media.bytes[511] != 0u)) failed |= 0x4000;
            }
            if (!failed) {
                core_machine_port_write(&machine->executor_port, 0x0322u, 0u);
                for (index = 0u; index < sizeof(sense_dcb); ++index)
                    core_machine_port_write(&machine->executor_port, 0x0320u, sense_dcb[index]);
                core_machine_hdc_advance(&machine->hdc);
                for (index = 0u; index < sizeof(sense); ++index) {
                    if (core_machine_port_read(&machine->executor_port, 0x0320u) != sense[index]) {
                        failed |= 0x8000;
                        break;
                    }
                }
            }
            if (!failed) {
                core_machine_port_write(&machine->executor_port, 0x0322u, 0u);
                for (index = 0u; index < sizeof(invalid_dcb); ++index)
                    core_machine_port_write(&machine->executor_port, 0x0320u, invalid_dcb[index]);
                core_machine_hdc_advance(&machine->hdc);
                if (core_machine_port_read(&machine->executor_port, 0x0320u) != 0x02u)
                    failed |= 0x10000;
            }
            if (!failed) {
                core_machine_port_write(&machine->executor_port, 0x0323u, 0x02u);
                core_machine_port_write(&machine->executor_port, 0x0322u, 0u);
                for (index = 0u; index < sizeof(dcb); ++index)
                    core_machine_port_write(&machine->executor_port, 0x0320u, dcb[index]);
                core_machine_hdc_advance(&machine->hdc);
                if (!core_machine_hdc_irq_pending(&machine->hdc) ||
                    core_machine_port_read(&machine->executor_port, 0x0320u) != response[0] ||
                    core_machine_hdc_irq_pending(&machine->hdc)) failed |= 0x20000;
            }
            if (!failed) {
                core_machine_port_write(&machine->executor_port, 0x0323u, 0u);
                core_machine_port_write(&machine->executor_port, 0x0322u, 0u);
                for (index = 0u; index < sizeof(read_dcb); ++index)
                    core_machine_port_write(&machine->executor_port, 0x0320u, read_dcb[index]);
                core_machine_hdc_advance(&machine->hdc);
                if (machine->hdc.xebec.phase != CORE_MACHINE_XEBEC_PHASE_DMA_READ ||
                    (machine->shared_dma_primary.data.status & VDMA_STATUS_DRQ(3u)) != 0u) {
                    failed |= 0x20000;
                }
                core_machine_port_write(&machine->executor_port, 0x0323u, 0x01u);
                if ((machine->shared_dma_primary.data.status & VDMA_STATUS_DRQ(3u)) == 0u) {
                    failed |= 0x20000;
                }
                dma_provider->terminal_count(&machine->hdc, &machine->shared_dma_latch);
                (C_VOID)core_machine_port_read(&machine->executor_port, 0x0320u);
            }
            if (!failed) {
                core_machine_port_write(&machine->executor_port, 0x0323u, 0x5au);
                if (machine->hdc.xebec.mask_pattern != 0x5au) failed |= 0x20000;
            }
            if (!failed) {
                core_machine_port_write(&machine->executor_port, 0x0322u, 0u);
                for (index = 0u; index < sizeof(initialize_dcb); ++index)
                    core_machine_port_write(&machine->executor_port, 0x0320u, initialize_dcb[index]);
                if (machine->hdc.xebec.phase != CORE_MACHINE_XEBEC_PHASE_INITIALIZE) failed |= 0x40000;
                for (index = 0u; index < sizeof(machine->hdc.xebec.initialize); ++index)
                    core_machine_port_write(&machine->executor_port, 0x0320u, 0u);
                core_machine_hdc_advance(&machine->hdc);
                if (machine->hdc.xebec.phase != CORE_MACHINE_XEBEC_PHASE_RESPONSE) failed |= 0x80000;
            }
            if (!failed) {
                core_machine_port_write(&machine->executor_port, 0x0321u, 0u);
                if (machine->hdc.xebec.dcb_count != 0u ||
                    machine->hdc.xebec.mask_pattern != 0u ||
                    machine->hdc.xebec.phase != CORE_MACHINE_XEBEC_PHASE_IDLE) failed |= 0x100000;
            }
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(registry);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T484:S15:XEBEC-STACK:FAIL bits=%x\n", failed);
        return 1;
    }
    puts("M5:T484:S15:XEBEC-STACK:OK");
    puts("M5:T484:S15:XEBEC-NO-ATA-ALIAS:OK");
    puts("M5:T484:S17:XEBEC-DMA-MEDIA:OK");
    puts("M5:T484:S18:XEBEC-DMA-RAM:OK");
    return 0;
}
