#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"

typedef struct core_machine_dma_fixture {
    type_unsigned_8 bytes[2];
    type_unsigned_8 next;
    C_UINT terminal_count;
} core_machine_dma_fixture;

static C_VOID core_machine_dma_fixture_read(C_VOID *owner, t_latch *latch)
{
    core_machine_dma_fixture *fixture = (core_machine_dma_fixture *)owner;

    if (fixture == STD_NULL || latch == STD_NULL) return;
    latch->data.byte = fixture->bytes[fixture->next++];
}

static C_VOID core_machine_dma_fixture_terminal(C_VOID *owner, t_latch *latch)
{
    core_machine_dma_fixture *fixture = (core_machine_dma_fixture *)owner;

    (C_VOID)latch;
    if (fixture != STD_NULL) ++fixture->terminal_count;
}

static C_VOID core_machine_dma_write_channel2(t_port *port, type_unsigned_16 address,
    type_unsigned_8 page, type_unsigned_16 count, type_unsigned_8 mode)
{
    core_machine_port_write(port, 0x000cu, 0u);
    core_machine_port_write(port, 0x0004u, address & 0xffu);
    core_machine_port_write(port, 0x0004u, address >> 8);
    core_machine_port_write(port, 0x0005u, count & 0xffu);
    core_machine_port_write(port, 0x0005u, count >> 8);
    core_machine_port_write(port, 0x0081u, page);
    core_machine_port_write(port, 0x000bu, mode);
}

static C_VOID core_machine_dma_write_primary_channel(t_port *port,
    type_unsigned_8 channel, type_unsigned_16 address, type_unsigned_16 count, type_unsigned_8 mode)
{
    type_unsigned_16 address_port = (type_unsigned_16)(channel * 2u);

    core_machine_port_write(port, 0x000cu, 0u);
    core_machine_port_write(port, address_port, address & 0xffu);
    core_machine_port_write(port, address_port, address >> 8u);
    core_machine_port_write(port, (type_unsigned_16)(address_port + 1u), count & 0xffu);
    core_machine_port_write(port, (type_unsigned_16)(address_port + 1u), count >> 8u);
    core_machine_port_write(port, 0x000bu, mode);
}

C_INT main(C_VOID)
{
    static const core_machine_dma_channel_provider provider = {
        core_machine_dma_fixture_read,
        STD_NULL,
        core_machine_dma_fixture_terminal
    };
    t_latch latch = {0};
    t_dma primary = {0};
    t_dma secondary = {0};
    t_ram memory = {0};
    t_port port;
    core_machine_dma_request_binding binding = {0};
    core_machine_dma_fixture fixture = {{0xa5u, 0x5au}, 0u, 0u};
    type_unsigned_8 bytes[2] = {0};
    type_unsigned_8 zeroes[2] = {0};
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_memory_initialize(&memory);
    if (core_machine_memory_allocate_for(&memory, 2u * 1024u * 1024u) !=
            TYPE_STATUS_OK ||
        core_machine_dma_bind_channel(&latch, &primary, &secondary, 2u,
            &provider, &fixture, &binding) != TYPE_STATUS_INVALID_STATE) {
        failed = 1;
        goto done;
    }
    core_machine_dma_initialize(&latch, &primary, &secondary, &port);
    core_machine_dma_reset(&latch, &primary, &secondary);
    if (core_machine_dma_bind_channel(&latch, &primary, &secondary, 2u,
            &provider, &fixture, &binding) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }

    /* Block-mode device -> RAM: count is inclusive, but each core DMA grant
     * may expose only one byte. */
    core_machine_dma_write_channel2(&port, 0x1234u, 0x01u, 1u, 0x86u);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_port_write(&port, 0x00d4u, 0x00u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11234u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0xa5u || bytes[1] != 0u || fixture.terminal_count != 0u ||
        (primary.data.status & VDMA_STATUS_TC(2u)) != 0u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) != 0u) {
        STD_FPRINTF(STD_STDERR,
            "DMA first: %02x %02x tc=%u status=%02x mask=%02x req=%02x slave=%02x/%02x\n",
            bytes[0], bytes[1], fixture.terminal_count, primary.data.status,
            primary.data.mask, primary.data.request, secondary.data.status,
            secondary.data.mask);
        failed = 1;
    }
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11234u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0xa5u || bytes[1] != 0x5au ||
        fixture.terminal_count != 1u ||
        (primary.data.status & VDMA_STATUS_TC(2u)) == 0u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) == 0u) {
        STD_FPRINTF(STD_STDERR,
            "DMA second: %02x %02x tc=%u status=%02x mask=%02x\n",
            bytes[0], bytes[1], fixture.terminal_count, primary.data.status,
            primary.data.mask);
        failed = 1;
    }

    /* A masked request and an explicit deassertion cannot move guest memory. */
    fixture.next = 0u;
    bytes[0] = bytes[1] = 0u;
    if (core_machine_memory_write_physical(&memory, 0x11234u,
            (type_virtual_address)zeroes, sizeof(zeroes)) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }
    core_machine_dma_write_channel2(&port, 0x1234u, 0x01u, 0u, 0x46u);
    core_machine_port_write(&port, 0x000au, 0x06u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11234u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0u) {
        STD_FPRINTF(STD_STDERR, "DMA masked: %02x status=%02x mask=%02x\n",
            bytes[0], primary.data.status, primary.data.mask);
        failed = 1;
    }
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_dma_request_deassert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11234u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0u) {
        STD_FPRINTF(STD_STDERR, "DMA deassert: %02x status=%02x mask=%02x\n",
            bytes[0], primary.data.status, primary.data.mask);
        failed = 1;
    }

    /* Auto-initialize reloads the programmed address/count after terminal
     * count, so a later request uses the same guest byte again. */
    fixture.bytes[0] = 0x3cu;
    fixture.bytes[1] = 0xc3u;
    fixture.next = 0u;
    core_machine_dma_write_channel2(&port, 0x1236u, 0x01u, 0u, 0x96u);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11236u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK ||
        bytes[0] != 0xc3u || primary.data.currAddr[2] != 0x1236u ||
        primary.data.currCount[2] != 0u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) != 0u) {
        STD_FPRINTF(STD_STDERR, "DMA auto-init: %02x addr=%04x count=%04x mask=%02x\n",
            bytes[0], primary.data.currAddr[2], primary.data.currCount[2],
            primary.data.mask);
        failed = 1;
    }

    /* Address-decrement changes only the core-owned current address. */
    fixture.bytes[0] = 0x7eu;
    fixture.next = 0u;
    core_machine_dma_write_channel2(&port, 0x1238u, 0x01u, 0u, 0xa6u);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11238u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK ||
        bytes[0] != 0x7eu || primary.data.currAddr[2] != 0x1237u) {
        STD_FPRINTF(STD_STDERR, "DMA decrement: %02x addr=%04x\n", bytes[0],
            primary.data.currAddr[2]);
        failed = 1;
    }

    /* Demand and single modes consume one request per grant. A second byte
     * requires a fresh device request, never an implicit block continuation. */
    fixture.bytes[0] = 0x11u;
    fixture.bytes[1] = 0x22u;
    fixture.next = 0u;
    fixture.terminal_count = 0u;
    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_dma_write_channel2(&port, 0x1240u, 0x01u, 1u, 0x06u);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_port_write(&port, 0x00d4u, 0x00u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11240u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0x11u || bytes[1] != 0u || fixture.terminal_count != 0u) {
        STD_FPRINTF(STD_STDERR, "DMA demand first: %02x %02x tc=%u\n",
            bytes[0], bytes[1], fixture.terminal_count);
        failed = 1;
    }
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11240u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[1] != 0x22u || fixture.terminal_count != 1u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) == 0u) {
        STD_FPRINTF(STD_STDERR, "DMA demand second: %02x %02x tc=%u mask=%02x\n",
            bytes[0], bytes[1], fixture.terminal_count, primary.data.mask);
        failed = 1;
    }

    fixture.bytes[0] = 0x33u;
    fixture.bytes[1] = 0x44u;
    fixture.next = 0u;
    fixture.terminal_count = 0u;
    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_dma_write_channel2(&port, 0x1242u, 0x01u, 1u, 0x46u);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_port_write(&port, 0x00d4u, 0x00u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11242u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0x33u || bytes[1] != 0u || fixture.terminal_count != 0u) {
        STD_FPRINTF(STD_STDERR, "DMA single first: %02x %02x tc=%u\n",
            bytes[0], bytes[1], fixture.terminal_count);
        failed = 1;
    }
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11242u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[1] != 0x44u || fixture.terminal_count != 1u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) == 0u) {
        STD_FPRINTF(STD_STDERR, "DMA single second: %02x %02x tc=%u mask=%02x\n",
            bytes[0], bytes[1], fixture.terminal_count, primary.data.mask);
        failed = 1;
    }

    /* Memory-to-memory uses the same grant boundary. Channel 0 is the
     * source request and channel 1 supplies the destination count. */
    bytes[0] = 0x55u;
    bytes[1] = 0x66u;
    if (core_machine_memory_write_physical(&memory, 0x0200u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        core_machine_memory_write_physical(&memory, 0x0300u,
            (type_virtual_address)zeroes, sizeof(zeroes)) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }
    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_dma_write_primary_channel(&port, 0u, 0x0200u, 1u, 0x80u);
    core_machine_dma_write_primary_channel(&port, 1u, 0x0300u, 1u, 0x81u);
    core_machine_port_write(&port, 0x0008u, VDMA_COMMAND_M2M);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_port_write(&port, 0x00d4u, 0u);
    core_machine_port_write(&port, 0x0009u, 0x04u);
    core_machine_port_write(&port, 0x0009u, 0x05u);
    core_machine_port_write(&port, 0x00d2u, 0x04u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x0300u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0x55u || bytes[1] != 0u ||
        (primary.data.status & VDMA_STATUS_TC(0u)) != 0u) {
        STD_FPRINTF(STD_STDERR, "DMA m2m first: %02x %02x status=%02x\n",
            bytes[0], bytes[1], primary.data.status);
        failed = 1;
    }
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x0300u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0x55u || bytes[1] != 0x66u ||
        (primary.data.status & VDMA_STATUS_TC(0u)) == 0u ||
        primary.data.request != 0u || primary.data.isr != 0u) {
        STD_FPRINTF(STD_STDERR, "DMA m2m second: %02x %02x status=%02x req=%02x isr=%02x\n",
            bytes[0], bytes[1], primary.data.status, primary.data.request,
            primary.data.isr);
        failed = 1;
    }

done:
    core_machine_dma_finalize(&latch, &primary, &secondary);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T269:S1:DMA-GRANT:PORT:OK\n");
    STD_PRINTF("M5:T269:S4:DMA-MODES:OK\n");
    STD_PRINTF("M5:T230:S3:DMA-CHANNEL:OK\n");
    return 0;
}
